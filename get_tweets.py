import os
import re
import threading
import time
import logging

from twitter import Api
from ConfigParser import ConfigParser
from OmegaExpansion import onionI2C


USER_IDS = ['1652541', '335455570', '5402612', '7424642', '546657547', '19527964', '2851607153']
curr_tweet = ["Waiting for tweets", "Still waiting", "Waiting more",]
curr_page = 0
paging_period = 4
data_lock = threading.Lock()

def load_config(filename):
    '''Load secrets from configfile'''
    config = ConfigParser()
    config.read(filename)
    return config


def split_the_tweet(tweet):
    global curr_page, curr_tweet
    with data_lock:
        try:
            curr_page = 0
            curr_tweet = []
            while len(tweet) > 32:
                page = tweet[:32]
                sp = page.rfind(" ")
                if sp > 0:
                    page = page[:sp]
                tweet = tweet[sp:]
                curr_tweet.append(page.strip())
            curr_tweet.append(tweet.strip())
            print(curr_tweet)
        except:
            logging.exception("Error in split_the_tweet")

def show():
    global curr_page, curr_tweet
    while True:
        if len(curr_tweet) > 0:
            if curr_page >= len(curr_tweet):
                curr_page = 0
            with data_lock:
                try:
                    msg = curr_tweet[curr_page]
                    data = [ord(c) for c in msg[:32]]
                    i2c.writeBytes(8, 2, data)
                    curr_page += 1
                except:
                    logging.exception("Error in show")
        time.sleep(paging_period)


if __name__ == '__main__':

        i2c     = onionI2C.OnionI2C()

        k = load_config("api_keys.ini")
        api = Api(k.get("twitter","consumer_key"),
                  k.get("twitter","consumer_secret"),
                  k.get("twitter","access_token"),
                  k.get("twitter","access_token_secret"))

        t = threading.Thread(target=show)
        t.daemon = True
        t.start()

        while True:
            try:
                for tweet in api.GetStreamFilter(follow=USER_IDS):
                        text = tweet.get("text")
                        if text is None:
                            continue
                        text = text.lower()[:140]
                        text = re.sub('http(s)?:[^\s]+', '', text)
                        text = re.sub('#', '', text)
                        text = re.sub('[\s]{2}', ' ', text).strip()
                        text = re.sub('.+:\s*', '', text)
                        if tweet.get("in_reply_to_screen_name") is not None or \
                           tweet.get("in_reply_to_status_id") is not None or \
                           tweet.get("retweeted_status") is not None or \
                           len(text) < 5:
                            continue

                        print text
                        split_the_tweet(text)
            except:
                logging.exception("Main loop")


