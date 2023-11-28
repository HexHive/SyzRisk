#!/usr/bin/python

class CommitData(object):
    def __init__(self, current_commit=None, fixed_commit=None,
                current_date=None, fixed_date=None, interval=None, vuln_type=None):
        self.current_commit= current_commit
        self.fixed_commit = fixed_commit
        self.current_date = current_date
        self.fixed_date = fixed_date
        self.interval = interval
        self.vuln_type = vuln_type
    

    def to_json(self):
        return{
            'current_commit' : self.current_commit,
            'fixed_commit'   : self.fixed_commit,
            'current_date'   : self.current_date,
            'fixed_date'     : self.fixed_date,
            'interval'       : self.interval,
            'vuln_type'      : self.vuln_type,
        }
    
    def __str__(self):
        return "%s;%s;%s;%s;%s;%s" % (self.current_commit, self.fixed_commit, 
                        self.current_date, self.fixed_date, self.interval, self.vuln_type)
    
    def __eq__(self, other):
        if isinstance(other, CommitData):
            return (self.current_commit == other.current_commit
                and self.fixed_commit == other.fixed_commit
                and self.current_date == other.current_date
                and self.fixed_date == other.fixed_date
                and self.interval == other.interval
                and self.vuln_type == other.vuln_type)
