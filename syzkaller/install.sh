wget https://dl.google.com/go/go1.18.7.linux-amd64.tar.gz
tar -xf go1.18.7.linux-amd64.tar.gz
export GOROOT=`pwd`/go
export PATH=$GOROOT/bin:$PATH
