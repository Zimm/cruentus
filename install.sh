#!/bin/bash
sudo mkdir -p /etc/cruentus/bin/
sudo cp ./build/cruentus /etc/cruentus/bin/
sudo rm /bin/cruentus &> /dev/null
sudo ln /etc/cruentus/bin/cruentus /bin/
