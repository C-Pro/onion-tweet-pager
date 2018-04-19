#!/usr/bin/env python
import os
import re
import threading
import time
import logging

from twitter import Api
from ConfigParser import ConfigParser
from OmegaExpansion import onionI2C

curr_tweet = ["Waiting for tweets"]
curr_page = 0
paging_period = 4
data_lock = threading.Lock()

def load_config(filename):
    '''Load secrets from configfile'''
    config = ConfigParser()
    config.read(filename)
    user_ids = config.get("twitter", "user_ids", "783214").split(",")
    config.set("twitter", "user_ids", user_ids)
    return config


def split_the_tweet(tweet):
    '''split tweet to 32 symbol pages'''
    curr_tweet = []
    tweet = chr(2) + " " + tweet # add start tweet symbol
    try:
        while len(tweet) > 32:
            page = tweet[:32]
            sp = page.rfind(" ")
            if sp > 0:
                page = page[:sp]
            tweet = tweet[sp:]
            curr_tweet.append(page.strip())
        curr_tweet.append(tweet.strip())
    except:
        logging.exception("Error in split_the_tweet")
    return(curr_tweet)

def send(tweet):
    '''send via i2c to arduino dock'''
    for page in tweet:
        data = [ord(c) for c in page[:32]]
        i2c.writeBytes(8, 2, data)
        time.sleep(paging_period)


if __name__ == '__main__':

        logging.basicConfig(format='%(asctime)s %(message)s',
                            level=logging.INFO)

        i2c     = onionI2C.OnionI2C()

        k = load_config("config.ini")
        api = Api(k.get("twitter","consumer_key"),
                  k.get("twitter","consumer_secret"),
                  k.get("twitter","access_token"),
                  k.get("twitter","access_token_secret"))

        while True:
            try:
                for tweet in api.GetStreamFilter(follow=k.get("twitter", "user_ids")):
                        text = tweet.get("text")
                        if text is None:
                            continue
                        text = text.lower()
                        text = re.sub('http(s)?:[^\s]+', '', text) # remove urls
                        text = re.sub('#', '', text) # remove hashtags
                        text = re.sub('[\s]{2}', ' ', text).strip() # trim extra spaces
                        # ignore replies and retweets
                        if tweet.get("in_reply_to_screen_name") is not None or \
                           tweet.get("in_reply_to_status_id") is not None or \
                           tweet.get("retweeted_status") is not None or \
                           len(text) < 5:
                            continue

                        logging.info(text)

                        send(split_the_tweet(text))
            except KeyboardInterrupt:
                print("Bye!")
                exit()
            except:
                logging.exception("Main loop")
                logging.warn("Waiting 60 seconds to avoid blocking twitter app")
                time.sleep(60)


