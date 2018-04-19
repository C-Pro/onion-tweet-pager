#/bin/sh
# Omega2 python build segfaults periodicaly
cd /root
trap "exit" SIGINT
while true
do
    python get_tweets.py
done

