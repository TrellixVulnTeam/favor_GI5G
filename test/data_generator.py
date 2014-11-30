# TODO: So this could eventually (and maybe should) just generate all our database seed data for consistency's sake, but
#when I wrote this, all I needed was messages - the other stuff was easy to handwrite - so for now that's all it does
from time import *
from datetime import *
from random import *
import os

TYPE_EMAIL = 0
TYPE_LINE = 2


class Address:
    def __init__(self, name, addrType):
        self.name = name
        self.addrType = addrType


class Account:
    def __init__(self, name, accountType):
        self.name = name
        self.accountType = accountType
        self.metrics = {}


ADDRESSES = [Address("test1@test.com", TYPE_EMAIL), Address("test2@test.com", TYPE_EMAIL),
             Address("test3@test.com", TYPE_EMAIL),
             Address("test4@test.com", TYPE_EMAIL), Address("Test1", TYPE_LINE), Address("Test2", TYPE_LINE)]

ACCOUNTS = [Account("account1@test.com", TYPE_EMAIL), Account("account2@test.com", TYPE_EMAIL),
            Account("account3", TYPE_LINE)]

TYPENAMES = {TYPE_EMAIL: "email", TYPE_LINE: "line"}

ID = 0


def rint(max):
    return int(random() * max)


#Account string, address object
def generate_row(account, addr):
    global ID

    if addr.name not in account.metrics:
        account.metrics[addr.name] = {}
        account.metrics[addr.name]["charcount"] = 0
        account.metrics[addr.name]["msgcount"] = 0

    msg_date = datetime.now()
    adjustment = timedelta(seconds=rint(59), minutes=rint(59), hours=rint(23), days=rint(6))
    if getrandbits(1):
        msg_date -= adjustment
    else:
        msg_date += adjustment

    msg_charcount = rint(1000)

    sql = 'INSERT INTO "' + account.name + "_" + TYPENAMES[addr.addrType] + "_"
    if account.metrics[addr.name]["msgcount"] % 2 == 0:
        sql += "sent"
    else:
        sql += "received"

    sql += '" VALUES(' + ",".join(
        str(x) for x in [ID, '"'+address.name+'"', int(msg_date.timestamp()), msg_charcount, getrandbits(1),
                         '"Test message body"']) + ");"
    ID += 1
    account.metrics[addr.name]["charcount"] += msg_charcount
    account.metrics[addr.name]["msgcount"] += 1
    return sql


def format_row(string):
    return '"'+string.replace('"', '\\"')+'"'+"\\\n"


if __name__ == '__main__':
    out = open('testdata.h', 'w')
    out.write('#define MESSAGE_TEST_DATA ')
    for account in ACCOUNTS:
        for address in ADDRESSES:
            if address.addrType == account.accountType:
                for i in range(40):
                    out.write(format_row(generate_row(account, address)))

    out.write("\"\"\n\n")
    for account in ACCOUNTS:
        out.write('//'+account.name + ":" + str(account.metrics)+"\n")
