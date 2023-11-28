// Copyright 2017 syzkaller project authors. All rights reserved.
// Use of this source code is governed by Apache 2 LICENSE that can be found in the LICENSE file.

// The test uses aetest package that starts local dev_appserver and handles all requests locally:
// https://cloud.google.com/appengine/docs/standard/go/tools/localunittesting/reference

package main

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"os"
	"os/exec"
	"path/filepath"
	"reflect"
	"runtime"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/google/go-cmp/cmp"
	"github.com/google/syzkaller/dashboard/dashapi"
	"golang.org/x/net/context"
	"google.golang.org/appengine/v2"
	"google.golang.org/appengine/v2/aetest"
	db "google.golang.org/appengine/v2/datastore"
	aemail "google.golang.org/appengine/v2/mail"
	"google.golang.org/appengine/v2/user"
)

type Ctx struct {
	t          *testing.T
	inst       aetest.Instance
	ctx        context.Context
	mockedTime time.Time
	emailSink  chan *aemail.Message
	client     *apiClient
	client2    *apiClient
}

var skipDevAppserverTests = func() bool {
	_, err := exec.LookPath("dev_appserver.py")
	// Don't silently skip tests on CI, we should have gcloud sdk installed there.
	return err != nil && os.Getenv("SYZ_BIG_ENV") == ""
}()

func NewCtx(t *testing.T) *Ctx {
	if skipDevAppserverTests {
		t.Skip("skipping test (no dev_appserver.py)")
	}
	t.Parallel()
	inst, err := aetest.NewInstance(&aetest.Options{
		// Without this option datastore queries return data with slight delay,
		// which fails reporting tests.
		StronglyConsistentDatastore: true,
	})
	if err != nil {
		t.Fatal(err)
	}
	r, err := inst.NewRequest("GET", "", nil)
	if err != nil {
		t.Fatal(err)
	}
	c := &Ctx{
		t:          t,
		inst:       inst,
		ctx:        appengine.NewContext(r),
		mockedTime: time.Date(2000, 1, 1, 0, 0, 0, 0, time.UTC),
		emailSink:  make(chan *aemail.Message, 100),
	}
	c.client = c.makeClient(client1, password1, true)
	c.client2 = c.makeClient(client2, password2, true)
	registerContext(r, c)
	return c
}

func (c *Ctx) expectOK(err error) {
	if err != nil {
		c.t.Helper()
		c.t.Fatal(err)
	}
}

func (c *Ctx) expectFail(msg string, err error) {
	c.t.Helper()
	if err == nil {
		c.t.Fatalf("expected to fail, but it does not")
	}
	if !strings.Contains(err.Error(), msg) {
		c.t.Fatalf("expected to fail with %q, but failed with %q", msg, err)
	}
}

func (c *Ctx) expectForbidden(err error) {
	c.t.Helper()
	if err == nil {
		c.t.Fatalf("expected to fail as 403, but it does not")
	}
	httpErr, ok := err.(HTTPError)
	if !ok || httpErr.Code != http.StatusForbidden {
		c.t.Fatalf("expected to fail as 403, but it failed as %v", err)
	}
}

func (c *Ctx) expectEQ(got, want interface{}) {
	if diff := cmp.Diff(got, want); diff != "" {
		c.t.Helper()
		c.t.Fatal(diff)
	}
}

func (c *Ctx) expectNE(got, want interface{}) {
	if reflect.DeepEqual(got, want) {
		c.t.Helper()
		c.t.Fatalf("equal: %#v", got)
	}
}

func (c *Ctx) expectTrue(v bool) {
	if !v {
		c.t.Helper()
		c.t.Fatal("failed")
	}
}

func caller(skip int) string {
	pcs := make([]uintptr, 10)
	n := runtime.Callers(skip+3, pcs)
	pcs = pcs[:n]
	frames := runtime.CallersFrames(pcs)
	stack := ""
	for {
		frame, more := frames.Next()
		if strings.HasPrefix(frame.Function, "testing.") {
			break
		}
		stack = fmt.Sprintf("%v:%v\n", filepath.Base(frame.File), frame.Line) + stack
		if !more {
			break
		}
	}
	if stack != "" {
		stack = stack[:len(stack)-1]
	}
	return stack
}

func (c *Ctx) Close() {
	if !c.t.Failed() {
		// To avoid per-day reporting limits for left-over emails.
		c.advanceTime(25 * time.Hour)
		// Ensure that we can render main page and all bugs in the final test state.
		_, err := c.GET("/test1")
		c.expectOK(err)
		_, err = c.GET("/test2")
		c.expectOK(err)
		_, err = c.GET("/test1/fixed")
		c.expectOK(err)
		_, err = c.GET("/test2/fixed")
		c.expectOK(err)
		_, err = c.GET("/admin")
		c.expectOK(err)
		var bugs []*Bug
		keys, err := db.NewQuery("Bug").GetAll(c.ctx, &bugs)
		if err != nil {
			c.t.Errorf("ERROR: failed to query bugs: %v", err)
		}
		for _, key := range keys {
			_, err = c.GET(fmt.Sprintf("/bug?id=%v", key.StringID()))
			c.expectOK(err)
		}
		// No pending emails (tests need to consume them).
		_, err = c.GET("/email_poll")
		c.expectOK(err)
		for len(c.emailSink) != 0 {
			c.t.Errorf("ERROR: leftover email: %v", (<-c.emailSink).Body)
		}
		// No pending external reports (tests need to consume them).
		resp, _ := c.client.ReportingPollBugs("test")
		for _, rep := range resp.Reports {
			c.t.Errorf("ERROR: leftover external report:\n%#v", rep)
		}
	}
	unregisterContext(c)
	c.inst.Close()
}

func (c *Ctx) advanceTime(d time.Duration) {
	c.mockedTime = c.mockedTime.Add(d)
}

// GET sends admin-authorized HTTP GET request to the app.
func (c *Ctx) GET(url string) ([]byte, error) {
	return c.AuthGET(AccessAdmin, url)
}

// AuthGET sends HTTP GET request to the app with the specified authorization.
func (c *Ctx) AuthGET(access AccessLevel, url string) ([]byte, error) {
	w, err := c.httpRequest("GET", url, "", access)
	if err != nil {
		return nil, err
	}
	return w.Body.Bytes(), nil
}

// POST sends admin-authorized HTTP POST requestd to the app.
func (c *Ctx) POST(url, body string) ([]byte, error) {
	w, err := c.httpRequest("POST", url, body, AccessAdmin)
	if err != nil {
		return nil, err
	}
	return w.Body.Bytes(), nil
}

// ContentType returns the response Content-Type header value.
func (c *Ctx) ContentType(url string) (string, error) {
	w, err := c.httpRequest("HEAD", url, "", AccessAdmin)
	return (w.Header()["Content-Type"][0]), err
}

func (c *Ctx) httpRequest(method, url, body string, access AccessLevel) (*httptest.ResponseRecorder, error) {
	c.t.Logf("%v: %v", method, url)
	r, err := c.inst.NewRequest(method, url, strings.NewReader(body))
	if err != nil {
		c.t.Fatal(err)
	}
	registerContext(r, c)
	if access == AccessAdmin || access == AccessUser {
		user := &user.User{
			Email:      "user@syzkaller.com",
			AuthDomain: "gmail.com",
		}
		if access == AccessAdmin {
			user.Admin = true
		}
		aetest.Login(user, r)
	}
	w := httptest.NewRecorder()
	http.DefaultServeMux.ServeHTTP(w, r)
	c.t.Logf("REPLY: %v", w.Code)
	if w.Code != http.StatusOK {
		return nil, HTTPError{w.Code, w.Body.String(), w.Result().Header}
	}
	return w, nil
}

type HTTPError struct {
	Code    int
	Body    string
	Headers http.Header
}

func (err HTTPError) Error() string {
	return fmt.Sprintf("%v: %v", err.Code, err.Body)
}

func (c *Ctx) loadBug(extID string) (*Bug, *Crash, *Build) {
	bug, _, err := findBugByReportingID(c.ctx, extID)
	if err != nil {
		c.t.Fatalf("failed to load bug: %v", err)
	}
	return c.loadBugInfo(bug)
}

func (c *Ctx) loadBugByHash(hash string) (*Bug, *Crash, *Build) {
	bug := new(Bug)
	bugKey := db.NewKey(c.ctx, "Bug", hash, 0, nil)
	c.expectOK(db.Get(c.ctx, bugKey, bug))
	return c.loadBugInfo(bug)
}

func (c *Ctx) loadBugInfo(bug *Bug) (*Bug, *Crash, *Build) {
	crash, _, err := findCrashForBug(c.ctx, bug)
	if err != nil {
		c.t.Fatalf("failed to load crash: %v", err)
	}
	build := c.loadBuild(bug.Namespace, crash.BuildID)
	return bug, crash, build
}

func (c *Ctx) loadJob(extID string) (*Job, *Build, *Crash) {
	jobKey, err := jobID2Key(c.ctx, extID)
	if err != nil {
		c.t.Fatalf("failed to create job key: %v", err)
	}
	job := new(Job)
	if err := db.Get(c.ctx, jobKey, job); err != nil {
		c.t.Fatalf("failed to get job %v: %v", extID, err)
	}
	build := c.loadBuild(job.Namespace, job.BuildID)
	crash := new(Crash)
	crashKey := db.NewKey(c.ctx, "Crash", "", job.CrashID, jobKey.Parent())
	if err := db.Get(c.ctx, crashKey, crash); err != nil {
		c.t.Fatalf("failed to load crash for job: %v", err)
	}
	return job, build, crash
}

func (c *Ctx) loadBuild(ns, id string) *Build {
	build, err := loadBuild(c.ctx, ns, id)
	c.expectOK(err)
	return build
}

func (c *Ctx) loadManager(ns, name string) (*Manager, *Build) {
	mgr, err := loadManager(c.ctx, ns, name)
	c.expectOK(err)
	build := c.loadBuild(ns, mgr.CurrentBuild)
	return mgr, build
}

func (c *Ctx) checkURLContents(url string, want []byte) {
	c.t.Helper()
	got, err := c.AuthGET(AccessAdmin, url)
	if err != nil {
		c.t.Fatalf("%v request failed: %v", url, err)
	}
	if !bytes.Equal(got, want) {
		c.t.Fatalf("url %v: got:\n%s\nwant:\n%s\n", url, got, want)
	}
}

func (c *Ctx) pollEmailBug() *aemail.Message {
	_, err := c.GET("/email_poll")
	c.expectOK(err)
	if len(c.emailSink) == 0 {
		c.t.Helper()
		c.t.Fatal("got no emails")
	}
	return <-c.emailSink
}

func (c *Ctx) expectNoEmail() {
	_, err := c.GET("/email_poll")
	c.expectOK(err)
	if len(c.emailSink) != 0 {
		msg := <-c.emailSink
		c.t.Helper()
		c.t.Fatalf("got unexpected email: %v\n%s", msg.Subject, msg.Body)
	}
}

func (c *Ctx) updRetestReproJobs() {
	_, err := c.GET("/retest_repros")
	c.expectOK(err)
}

type apiClient struct {
	*Ctx
	*dashapi.Dashboard
}

func (c *Ctx) makeClient(client, key string, failOnErrors bool) *apiClient {
	doer := func(r *http.Request) (*http.Response, error) {
		registerContext(r, c)
		w := httptest.NewRecorder()
		http.DefaultServeMux.ServeHTTP(w, r)
		res := &http.Response{
			StatusCode: w.Code,
			Status:     http.StatusText(w.Code),
			Body:       ioutil.NopCloser(w.Result().Body),
		}
		return res, nil
	}
	logger := func(msg string, args ...interface{}) {
		c.t.Logf("%v: "+msg, append([]interface{}{caller(3)}, args...)...)
	}
	errorHandler := func(err error) {
		if failOnErrors {
			c.t.Fatalf("\n%v: %v", caller(2), err)
		}
	}
	dash, err := dashapi.NewCustom(client, "", key, c.inst.NewRequest, doer, logger, errorHandler)
	if err != nil {
		panic(fmt.Sprintf("Impossible error: %v", err))
	}
	return &apiClient{
		Ctx:       c,
		Dashboard: dash,
	}
}

func (client *apiClient) pollBugs(expect int) []*dashapi.BugReport {
	resp, _ := client.ReportingPollBugs("test")
	if len(resp.Reports) != expect {
		client.t.Helper()
		client.t.Fatalf("want %v reports, got %v", expect, len(resp.Reports))
	}
	for _, rep := range resp.Reports {
		reproLevel := dashapi.ReproLevelNone
		if len(rep.ReproC) != 0 {
			reproLevel = dashapi.ReproLevelC
		} else if len(rep.ReproSyz) != 0 {
			reproLevel = dashapi.ReproLevelSyz
		}
		reply, _ := client.ReportingUpdate(&dashapi.BugUpdate{
			ID:         rep.ID,
			JobID:      rep.JobID,
			Status:     dashapi.BugStatusOpen,
			ReproLevel: reproLevel,
			CrashID:    rep.CrashID,
		})
		client.expectEQ(reply.Error, false)
		client.expectEQ(reply.OK, true)
	}
	return resp.Reports
}

func (client *apiClient) pollBug() *dashapi.BugReport {
	return client.pollBugs(1)[0]
}

func (client *apiClient) pollNotifs(expect int) []*dashapi.BugNotification {
	resp, _ := client.ReportingPollNotifications("test")
	if len(resp.Notifications) != expect {
		client.t.Helper()
		client.t.Fatalf("want %v notifs, got %v", expect, len(resp.Notifications))
	}
	return resp.Notifications
}

func (client *apiClient) updateBug(extID string, status dashapi.BugStatus, dup string) {
	reply, _ := client.ReportingUpdate(&dashapi.BugUpdate{
		ID:     extID,
		Status: status,
		DupOf:  dup,
	})
	client.expectTrue(reply.OK)
}

func (client *apiClient) pollSpecificJobs(manager string, jobs dashapi.ManagerJobs) *dashapi.JobPollResp {
	req := &dashapi.JobPollReq{
		Managers: map[string]dashapi.ManagerJobs{
			manager: jobs,
		},
	}
	resp, err := client.JobPoll(req)
	client.expectOK(err)
	return resp
}

func (client *apiClient) pollJobs(manager string) *dashapi.JobPollResp {
	return client.pollSpecificJobs(manager, dashapi.ManagerJobs{
		TestPatches: true,
		BisectCause: true,
		BisectFix:   true,
	})
}

func (client *apiClient) pollAndFailBisectJob(manager string) {
	resp := client.pollJobs(manager)
	client.expectNE(resp.ID, "")
	client.expectEQ(resp.Type, dashapi.JobBisectCause)
	done := &dashapi.JobDoneReq{
		ID:    resp.ID,
		Error: []byte("pollAndFailBisectJob"),
	}
	client.expectOK(client.JobDone(done))
}

type (
	EmailOptMessageID int
	EmailOptSubject   string
	EmailOptFrom      string
	EmailOptCC        []string
)

func (c *Ctx) incomingEmail(to, body string, opts ...interface{}) {
	id := 0
	subject := "crash1"
	from := "default@sender.com"
	cc := []string{"test@syzkaller.com", "bugs@syzkaller.com", "bugs2@syzkaller.com"}
	for _, o := range opts {
		switch opt := o.(type) {
		case EmailOptMessageID:
			id = int(opt)
		case EmailOptSubject:
			subject = string(opt)
		case EmailOptFrom:
			from = string(opt)
		case EmailOptCC:
			cc = []string(opt)
		}
	}
	email := fmt.Sprintf(`Sender: %v
Date: Tue, 15 Aug 2017 14:59:00 -0700
Message-ID: <%v>
Subject: %v
From: %v
Cc: %v
To: %v
Content-Type: text/plain

%v
`, from, id, subject, from, strings.Join(cc, ","), to, body)
	_, err := c.POST("/_ah/mail/", email)
	c.expectOK(err)
}

func initMocks() {
	// Mock time as some functionality relies on real time.
	timeNow = func(c context.Context) time.Time {
		return getRequestContext(c).mockedTime
	}
	sendEmail = func(c context.Context, msg *aemail.Message) error {
		getRequestContext(c).emailSink <- msg
		return nil
	}
}

// Machinery to associate mocked time with requests.
type RequestMapping struct {
	c   context.Context
	ctx *Ctx
}

var (
	requestMu       sync.Mutex
	requestContexts []RequestMapping
)

func registerContext(r *http.Request, c *Ctx) {
	requestMu.Lock()
	defer requestMu.Unlock()
	requestContexts = append(requestContexts, RequestMapping{appengine.NewContext(r), c})
}

func getRequestContext(c context.Context) *Ctx {
	requestMu.Lock()
	defer requestMu.Unlock()
	for _, m := range requestContexts {
		if reflect.DeepEqual(c, m.c) {
			return m.ctx
		}
	}
	panic(fmt.Sprintf("no context for: %#v", c))
}

func unregisterContext(c *Ctx) {
	requestMu.Lock()
	defer requestMu.Unlock()
	n := 0
	for _, m := range requestContexts {
		if m.ctx == c {
			continue
		}
		requestContexts[n] = m
		n++
	}
	requestContexts = requestContexts[:n]
}
