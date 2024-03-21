FROM debian:latest

RUN apt update && apt install -y nmap fish valgrind cmake build-essential clang curl git make libpcap-dev openssh-server tcpdump rsync
RUN mkdir -p /ft_nmap/src && mkdir -p /ft_nmap/inc

RUN echo 'root:1234' | chpasswd


RUN (echo 'LogLevel QUIET';    echo 'PermitRootLogin yes';    echo 'PasswordAuthentication yes';  echo 'Subsystem sftp /usr/lib/openssh/sftp-server'; ) > /etc/ssh/sshd_config
RUN mkdir -p /run/sshd

WORKDIR /ft_nmap
CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config"]