"""Jinja2 environment getters
"""


from jinja2 import Environment, FileSystemLoader
from .filters import gen_id

def __make_env(include_paths):
    env = Environment(
        loader=FileSystemLoader(include_paths)
    )
    env.filters['genid'] = gen_id
    return env

def get_env(include_paths=None):
    """Get Jinja2 environment"""
    if not hasattr(get_env, 'env'):
        setattr(get_env, 'env', __make_env(include_paths))
    env = getattr(get_env, 'env')
    return env
