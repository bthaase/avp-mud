FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive 

RUN apt-get update && apt-get install -y --allow-downgrades make  \
                    g++ libz-dev gdb valgrind locales gdbserver \
                    libsystemd0 libsystemd-dev libmosquitto-dev 

RUN sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && locale-gen

ENV LC_ALL en_US.UTF-8     

RUN mkdir /opt/avp-mud

WORKDIR /opt/avp-mud

COPY src /opt/avp-mud/src
copy run.sh /opt/avp-mud/run.sh

RUN mkdir /opt/avp-mud/src/o
WORKDIR /opt/avp-mud/src
RUN make -j8

WORKDIR /opt/avp-mud/area

EXPOSE 7676

# RUN echo "set print thread-events off" >> /root/.gdbinit

# RUN echo "set print inferior-events off" >> /root/.gdbinit

CMD ["/opt/avp-mud/src/run_under_docker.sh", "/opt/avp-mud/src/avp", "7676"]
