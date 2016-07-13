from time import *
from datetime import *
from random import *
import os
import json

TYPE_EMAIL = 0
TYPE_LINE = 2

TYPENAMES = {TYPE_EMAIL: "email", TYPE_LINE: "line"}
TYPEDEFS = {TYPE_EMAIL: "TYPE_EMAIL", TYPE_LINE: "TYPE_LINE"}

MSG_COUNT = 42 #This should be an an odd number x2: (2k+1)x2 = 4k+2, so each table has an odd number of messages


unix_week = int(datetime.now().timestamp() - (((datetime.now() - timedelta(weeks=1)).timestamp())))

MSG_MODIFIERS = []


#TODO: we can generate initializer lists for the "definedX" vectors used in testing too, and that'd make things much easier

def list_to_string(list):
    return ",".join(str(x) for x in list)


def format_row(string):
    return '"' + string.replace('"', '\\"') + '"' + "\\\n"


def format_defstring(string):
    return string.replace("@", "_at_").replace(".", "_dot_")


def rint(max):
    return int(random() * max)


class Track:
    IDS = {}
    MAPS = {}
    MAXES = {}


    def __init__(self, name, cls):
        self.name = name
        if cls not in Track.IDS:
            Track.IDS[cls] = 0

        if cls not in Track.MAPS:
            Track.MAPS[cls] = {}

        if name in Track.MAPS[cls]:
            self.id = Track.MAPS[cls][name].id
        else:
            Track.IDS[cls] += 1
            self.id = Track.IDS[cls]
            Track.MAPS[cls][name] = self

    def self.maxCheck()


class Address(Track):
    def __init__(self, name, addrType, count):
        super().__init__(name, self.__class__)
        self.addrType = addrType
        self.count = count
        self.contact = None

    def sql(self):
        if self.contact:
            contact_id = self.contact.id
        else:
            contact_id = "NULL"
        return "INSERT INTO addresses_" + TYPENAMES[self.addrType] + " VALUES (" + list_to_string(
            ['"' + self.name + '"',
             self.count,
             contact_id]) + ");"

    def args(self):
        cid = -1
        if self.contact:
            cid = self.contact.id
        return list_to_string(['"' + self.name + '"', self.count, cid, TYPEDEFS[self.addrType]])

    def defargs(self):
        return "#define ADDR_" + format_defstring(self.name) + "_ARGS (" + self.args() + ")\n"


class Contact(Track):
    def __init__(self, name, count):
        #TODO: count looks an awful lot like an unused/irrelevant parameter
        super().__init__(name, self.__class__)
        self.addresses = []


    def add_address(self, account):
        account.contact = self
        self.addresses.append(account)

    def flag(self):
        active_types = {}
        for address in self.addresses:
            active_types[address.addrType] = True
        flag = 0
        for key in active_types:
            flag += 2 ** key
        return flag


    def sql(self):
        return "INSERT INTO contacts VALUES(" + list_to_string([self.id, '"' + self.name + '"']) + ");"
        #return "INSERT INTO contacts VALUES(" + list_to_string([self.id, '"' + self.name + '"', self.flag()]) + ");"

    def defargs(self):
       # arglist = [self.id, '"' + self.name + '"', "(MessageTypeFlag)" + str(self.flag())]
        arglist = [self.id, '"' + self.name + '"']
        if len(self.addresses) > 0:
            addr_vector = "{"
            a = ["Address(" + x.args() + ")" for x in self.addresses]
            addr_vector += ",".join(a)
            addr_vector += "}"
            arglist.append(addr_vector)
        return "#define CONTACT_" + format_defstring(self.name) + "_ARGS (" + list_to_string(arglist) + ")\n"


class Account(Track):
    def __init__(self, name, accountType):
        super().__init__(name, self.__class__)
        self.accountType = accountType
        self.messages = {}
        self.metrics = {}
        self.metrics["overall"] = {}
        self.metrics["overall"]["charcount_sent"] = 0
        self.metrics["overall"]["msgcount_sent"] = 0
        self.metrics["overall"]["charcount_received"] = 0
        self.metrics["overall"]["msgcount_received"] = 0

    def json_escaped(self):
        return '"' + (json.dumps({"password": "no", "url": "imap://imap.no.com"}).replace('"', "\\\"")) + '"'

    def json_insert(self):
        return "'" + (json.dumps({"password": "no", "url": "imap://imap.no.com"}).replace('"', "\"")) + "'"

    def sql(self):
        return "INSERT INTO accounts VALUES (" + list_to_string(['"' + self.name + '"', self.accountType,
                                                                 self.json_insert()]) + ");"

    def senttable(self):
        return "\"CREATE TABLE \\\"" + self.name + "_" + TYPENAMES[
            self.accountType] + "_sent\\\"\" SENT_TABLE_SCHEMA \";\"\\\n"

    def rectable(self):
        return "\"CREATE TABLE \\\"" + self.name + "_" + TYPENAMES[
            self.accountType] + "_received\\\"\" RECEIVED_TABLE_SCHEMA \";\"\\\n"

    def defargs(self):
        return "#define ACC_" + format_defstring(self.name) + "_ARGS (" + list_to_string(
            ['"' + self.name + '"', TYPEDEFS[self.accountType],
             self.json_escaped()]) + ")\n"
    def outname(self):
        return "#define ACC_" + format_defstring(self.name) + "_NAME \"" + self.name + "\"\n"


MSG_ID = 0


def setup_msg_modifiers():
    #Testing is much easier if the message date modifiers are unique so we don't get weird edge cases with date limitations
    global MSG_MODIFIERS
    msg_total = 0
    for account in Track.MAPS[Account].values():
        for address in Track.MAPS[Address].values():
            if address.addrType == account.accountType:
                msg_total += MSG_COUNT
    MSG_MODIFIERS = sample(range(1, unix_week), msg_total)

def init():
    "WARNING: IF YOU CHANGE THE DEFINITIONS IN THIS METHOD AND REGENERATE THE DATA, TESTS RELYING ON SPECIFIC DATABASE \
    CONFIGURATIONS (THERE ARE MANY) MAY CEASE TO COMPILE AND/OR SUCCEED. PLEASE PROCEED CAREFULLY."

    con1 = Contact("EmailTest1", 1)  #Email
    con1.add_address(Address("test3@test.com", TYPE_EMAIL, 1))

    con2 = Contact("LineTest2", 4)  #Line
    con2.add_address(Address("Test1", TYPE_LINE, 2))

    con3 = Contact("LineEmailTest3", 5)  #Line + Email
    con3.add_address(Address("Test2", TYPE_LINE, 1))
    con3.add_address(Address("test4@test.com", TYPE_EMAIL, 2))

    con4 = Contact("TwoEmailTest4", 1)
    con4.add_address(Address("dubtest1@test.com", TYPE_EMAIL, 5))
    con4.add_address(Address("dubtest2@test.com", TYPE_EMAIL, 6))

    #There needs to be at least one address not tied to a contact so we can test faculties for that specifically
    Address("test1@test.com", TYPE_EMAIL, 1)
    #Address("test2@test.com", TYPE_EMAIL, 2) #Skipping this right now to slim down the data a bit

    #Contact("EmailTest4", 1) #Contacts without addresses are basically meaningless, skipping these for now
    #Contact("EmailTest5", 1)

    Account("account1@test.com", TYPE_EMAIL)
    Account("account2@test.com", TYPE_EMAIL)
    Account("account3", TYPE_LINE)

    setup_msg_modifiers()





#Account string, address object
def generate_row(account, addr):
    global MSG_ID
    global MSG_MODIFIERS

    if addr.name not in account.metrics:
        account.metrics[addr.name] = {}
        account.metrics[addr.name]["charcount_sent"] = 0
        account.metrics[addr.name]["msgcount_sent"] = 0
        account.metrics[addr.name]["charcount_received"] = 0
        account.metrics[addr.name]["msgcount_received"] = 0
        account.messages[addr.name] = {}
        account.messages[addr.name][True] = []
        account.messages[addr.name][False] = []

    msg_date = int(datetime.now().timestamp())
    adjustment = MSG_MODIFIERS.pop()
    if getrandbits(1):
        msg_date -= adjustment
    else:
        msg_date += adjustment

    #print(datetime.fromtimestamp(msg_date).strftime('%Y-%m-%d %H:%M:%S'))
    msg_charcount = rint(1000)

    sql = 'INSERT INTO "' + account.name + "_" + TYPENAMES[addr.addrType] + "_"
    if (account.metrics[addr.name]["msgcount_sent"] + account.metrics[addr.name]["msgcount_received"]) % 2 == 0:
        sql += "sent"
        sent = True
        account.metrics[addr.name]["charcount_sent"] += msg_charcount
        account.metrics[addr.name]["msgcount_sent"] += 1
        account.metrics["overall"]["charcount_sent"] += msg_charcount
        account.metrics["overall"]["msgcount_sent"] += 1
    else:
        sql += "received"
        sent = False
        account.metrics[addr.name]["charcount_received"] += msg_charcount
        account.metrics[addr.name]["msgcount_received"] += 1
        account.metrics["overall"]["charcount_received"] += msg_charcount
        account.metrics["overall"]["msgcount_received"] += 1

    media = getrandbits(1)
    account.messages[addr.name][sent].append((MSG_ID, address.name, msg_date, msg_charcount, bool(media)))

    sql += '" VALUES(' + ",".join(
        str(x) for x in [MSG_ID, '"' + address.name + '"', msg_date, msg_charcount, media,
                         '"Test message body"']) + ");"
    MSG_ID += 1
    return sql


def process_msglist(msgs, name, out):
    mindate = min(x[2] for x in msgs)
    maxdate = max(x[2] for x in msgs)
    middate = sorted(msgs, key=lambda x: x[2])[int(len(msgs)/2)][2]
    #print(str([x[2] for x in sorted(msgs, key=lambda x: x[2])])+str(middate))
    out.write("#define "+format_defstring(name).upper()+"_MINDATE "+str(mindate)+"\n")
    out.write("#define "+format_defstring(name).upper()+"_MIDDATE "+str(middate)+"\n")
    out.write("#define "+format_defstring(name).upper()+"_MAXDATE "+str(maxdate)+"\n")
    msgs = sorted(msgs, key=lambda x: x[2])
    date_list = [x[2] for x in msgs]
    charcount_list = [x[3] for x in msgs]
    address_list = [x[1] for x in msgs]
    out.write("#define "+format_defstring(name).upper()+"_DATELIST_ARG "+str(date_list).replace("[", "{").replace("]", "}")+"\n")
    out.write("#define "+format_defstring(name).upper()+"_ADDRESSLIST_ARG "+
              str(address_list).replace("[", "{").replace("]", "}").replace("'", '"')+"\n")
    out.write("#define "+format_defstring(name).upper()+"_CHARCOUNTLIST_ARG "+str(charcount_list).replace("[", "{").replace("]", "}")+"\n")
    example_msg = msgs[0]
    out.write("#define "+format_defstring(name).upper()+"_EXAMPLE_ID "+str(msgs[0][0])+"\n")
    out.write("#define "+format_defstring(name).upper()+"_EXAMPLE_ADDR \"" +str(msgs[0][1])+"\"\n")
    out.write("#define "+format_defstring(name).upper()+"_EXAMPLE_DATE " +str(msgs[0][2])+"\n")
    out.write("#define "+format_defstring(name).upper()+"_EXAMPLE_CHARCOUNT " +str(msgs[0][3])+"\n")
    out.write("#define "+format_defstring(name).upper()+"_EXAMPLE_SENT " +str(1 if msgs[0][4] else 0)+"\n")

if __name__ == '__main__':
    init()
    block_end = "\"\"\n\n"

    #Write the message data
    out = open('testdata.h', 'w')
    out.write('#define MESSAGE_TEST_DATA ')
    for account in Track.MAPS[Account].values():
        for address in Track.MAPS[Address].values():
            if address.addrType == account.accountType:
                for i in range(MSG_COUNT):
                    out.write(format_row(generate_row(account, address)))

    out.write(block_end)

    #Write the summation data about messages
    for account in Track.MAPS[Account].values():
        out.write('//' + account.name + ":" + str(account.metrics) + "\n")
        for address in account.messages:
            process_msglist(account.messages[address][True], account.name+"_"+address+"_SENT", out)
            process_msglist(account.messages[address][False], account.name+"_"+address+"_RECEIVED", out)
        for address in account.metrics:
            for metric in account.metrics[address]:
                definition = format_defstring(account.name) + "_" + format_defstring(address) + "_" + metric
                definition = definition.upper()
                out.write("#define " + definition + " " + str(account.metrics[address][metric]) + "\n")

        out.write("\n")

    #Write out the data to generate addresses
    out.write('#define ADDRESS_TEST_DATA ')
    for address in Track.MAPS[Address].values():
        out.write(format_row(address.sql()))

    out.write(block_end)

    #Write out the data to generate contacts
    out.write('#define CONTACT_TEST_DATA ')
    for contact in Track.MAPS[Contact].values():
        out.write(format_row(contact.sql()))

    out.write(block_end)

    #Write out the data to generate accounts
    out.write('#define ACCOUNT_TEST_DATA ')
    for account in Track.MAPS[Account].values():
        out.write(format_row(account.sql()))
        out.write(account.senttable())
        out.write(account.rectable())

    out.write(block_end)

    for address in Track.MAPS[Address].values():
        out.write(address.defargs())
    out.write("\n\n")
    for contact in Track.MAPS[Contact].values():
        out.write(contact.defargs())
    out.write("\n\n")
    for account in Track.MAPS[Account].values():
        out.write(account.defargs())
        out.write(account.outname())
    out.write("\n\n")


    out.write("#define MSG_COUNT "+str(MSG_COUNT)+"\n")
    out.write("#define CONTACT_COUNT "+str(len(Track.MAPS[Contact]))+"\n")
    out.write("#define ADDR_COUNT "+str(len(Track.MAPS[Address]))+"\n")
    out.write("#define ACCOUNT_COUNT "+str(len(Track.MAPS[Account]))+"\n")
    out.close()
