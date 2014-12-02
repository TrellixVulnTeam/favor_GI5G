# TODO: So this could eventually (and maybe should) just generate all our database seed data for consistency's sake, but
#when I wrote this, all I needed was messages - the other stuff was easy to handwrite - so for now that's all it does
from time import *
from datetime import *
from random import *
import os

TYPE_EMAIL = 0
TYPE_LINE = 2


TYPENAMES = {TYPE_EMAIL: "email", TYPE_LINE: "line"}


def list_to_string(list):
    return ",".join(str(x) for x in list)



class Track:
    IDS = {}
    MAPS = {}


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
        return "INSERT INTO addresses_"+TYPENAMES[self.addrType]+" VALUES ("+list_to_string(['"'+self.name+'"',
                                                                                            self.count,
                                                                                            contact_id])+");"


class Contact(Track):

    def __init__(self, name, count):
        super().__init__(name, self.__class__)
        self.addresses = []


    def add_address(self, account):
        account.contact = self
        self.addresses.append(account)


    def sql(self):
        #This is only a little complicated because we have to translate types into keys
        active_types = {}
        for address in self.addresses:
            active_types[address.addrType] = True

        flag = 0
        for key in active_types:
            flag += 2**key
        return "INSERT INTO contacts VALUES("+list_to_string([self.id, '"'+self.name+'"', flag])+");"


class Account(Track):

    def __init__(self, name, accountType):
        super().__init__(name, self.__class__)
        self.accountType = accountType
        self.metrics = {}

    def sql(self):
        return "TODO :("

MSG_ID = 0

def init():
    con1 = Contact("EmailTest1", 1) #Email
    con1.add_address(Address("test3@test.com", TYPE_EMAIL, 1))

    con2 = Contact("LineTest2", 4) #Line
    con2.add_address(Address("Test1", TYPE_LINE, 2))

    con3 = Contact("LineEmailTest3", 5) #Line + Email
    con3.add_address(Address("Test2", TYPE_LINE, 1))
    con3.add_address(Address("test4@test.com", TYPE_EMAIL, 2))

    #So they live in the class list
    Address("test1@test.com", TYPE_EMAIL, 1)
    #Address("test2@test.com", TYPE_EMAIL, 2) #Skipping this right now to slim down the data a bit

    #Contact("EmailTest4", 1) #Contacts without addresses are basically meaningless, skipping these for now
    #Contact("EmailTest5", 1)

    Account("account1@test.com", TYPE_EMAIL)
    Account("account2@test.com", TYPE_EMAIL)
    Account("account3", TYPE_LINE)



def rint(max):
    return int(random() * max)


#Account string, address object
def generate_row(account, addr):
    global MSG_ID

    if addr.name not in account.metrics:
        account.metrics[addr.name] = {}
        account.metrics[addr.name]["charcount_sent"] = 0
        account.metrics[addr.name]["msgcount_sent"] = 0
        account.metrics[addr.name]["charcount_received"] = 0
        account.metrics[addr.name]["msgcount_received"] = 0


    msg_date = datetime.now()
    adjustment = timedelta(seconds=rint(59), minutes=rint(59), hours=rint(23), days=rint(6))
    if getrandbits(1):
        msg_date -= adjustment
    else:
        msg_date += adjustment

    msg_charcount = rint(1000)

    sql = 'INSERT INTO "' + account.name + "_" + TYPENAMES[addr.addrType] + "_"
    if (account.metrics[addr.name]["msgcount_sent"] + account.metrics[addr.name]["charcount_received"]) % 2 == 0:
        sql += "sent"
        account.metrics[addr.name]["charcount_sent"] += msg_charcount
        account.metrics[addr.name]["msgcount_sent"] += 1
    else:
        sql += "received"
        account.metrics[addr.name]["charcount_received"] += msg_charcount
        account.metrics[addr.name]["msgcount_received"] += 1

    sql += '" VALUES(' + ",".join(
        str(x) for x in [MSG_ID, '"'+address.name+'"', int(msg_date.timestamp()), msg_charcount, getrandbits(1),
                         '"Test message body"']) + ");"
    MSG_ID += 1
    return sql


def format_row(string):
    return '"'+string.replace('"', '\\"')+'"'+"\\\n"

def format_defstring(string):
    return string.replace("@", "_at_").replace(".", "_dot_")


if __name__ == '__main__':
    init()
    block_end = "\"\"\n\n"

    #Write the message data
    out = open('testdata.h', 'w')
    out.write('#define MESSAGE_TEST_DATA ')
    for account in Track.MAPS[Account].values():
        for address in Track.MAPS[Address].values():
            if address.addrType == account.accountType:
                for i in range(40):
                    out.write(format_row(generate_row(account, address)))

    out.write(block_end)

    #Write the summation data about messages
    for account in Track.MAPS[Account].values():
        out.write('//'+account.name + ":" + str(account.metrics)+"\n")
        for address in account.metrics:
            for metric in account.metrics[address]:
                definition = format_defstring(account.name)+"_"+format_defstring(address)+"_"+metric
                definition = definition.upper()
                out.write("#define "+definition+" "+str(account.metrics[address][metric])+"\n")

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


    out.close()
