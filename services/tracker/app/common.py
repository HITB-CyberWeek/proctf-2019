import hashlib
import logging
import secrets
import uuid

import asyncpg

from app.config import get_config


def bad_request(message="Bad request"):
    return message, 400


def forbidden(message="Forbidden"):
    return message, 403


def generate_secret():
    return "s-" + secrets.token_hex(16)


def generate_token():
    return "t-" + secrets.token_hex(8)


def hash_password(password):
    return hashlib.sha256(password.encode("utf-8")).hexdigest()


def is_uuid(value):
    try:
        uuid.UUID(value)
    except ValueError:
        return False
    return True


def configure_logging():
    logging.basicConfig(format="%(asctime)s %(levelname)8s %(message)s",
                        level=logging.DEBUG)


async def connect_db():
    config = get_config()
    return await asyncpg.connect(config.database_uri)


def handler(request_id: int):
    def wrapper(func):
        func.request_id = request_id
        return func
    return wrapper
