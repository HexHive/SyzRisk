#!/bin/bash 

set -e

CUR_DIR=$(realpath $(dirname $BASH_SOURCE))

echo "[*] Checking dependencies..."

if [[ -z `which unzip` ]]; then 
  echo "info: installing 'unzip'..."
  sudo apt install unzip
fi
if [[ -z `which java` ]]; then 
  echo "info: installing 'java'..."
  sudo apt install openjdk-11-jdk
fi
if [[ -z `which python` ]]; then
  echo "info: installing 'python'..."
  sudo apt install python3
  sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10
  sudo update-alternatives --set python /usr/bin/python3
fi
sudo pip install GitPython alive-progress progressbar2 numpy termcolor


### Joern (v1.1.360)

JOERN_VERSION="v1.1.360"

echo "[*] Installing Joern $JOERN_VERSION..."

curl -L "https://github.com/joernio/joern/releases/download/$JOERN_VERSION/joern-install.sh" -o joern-install.sh
chmod u+x joern-install.sh
./joern-install.sh --install-dir=joern --version=$JOERN_VERSION
rm joern-install.sh


echo "[+] Done. Don't forget to source SOURCE_ME!"
