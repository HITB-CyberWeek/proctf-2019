FROM ubuntu
RUN apt update --fix-missing
RUN apt install -y python3 python3-flask
RUN apt install -y libseccomp-dev gcc
RUN apt install -y libssl-dev
WORKDIR /home/checker
RUN apt install -y python3-pip
RUN pip3 install requests paramiko
RUN apt install -y nasm
CMD ./checker.py
