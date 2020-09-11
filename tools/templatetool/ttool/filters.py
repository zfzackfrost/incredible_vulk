"""Jinja2 filters"""


import random
import string

def gen_id(value, count=16):
    """Jinja2 filter to generate a random ID string"""
    random.seed(value)
    letters = string.ascii_uppercase
    return ''.join(random.choice(letters) for i in range(count))
