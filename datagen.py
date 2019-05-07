#!/usr/bin/python3

import json

DATA_JSON_FILENAME = "database.json"
DATA_HEADER_FILENAME = "testdata.h"
WARNING_MSG = """
/****************************************************************************
*****************************************************************************
** Be very careful! This is an autogenerated file so any changes you
** make will likely be erased. If you want to add fields or other information
** add it to the data.json document, then run 'python datagen.py' to rebuild
** this file
****************************************************************************
****************************************************************************/\n\n
"""

def load_schema():
    schema = {}
    with open(DATA_JSON_FILENAME, "r") as f:
        schema = json.load(f)
    
    return schema

# Send a key and get a dict back
def search_schema(src, target):
    if type(src) != dict:
        return None
    
    ret = False
    for key, val in src.items():    
        if str(key) == target:
            return val
         
        ret = search_schema(val,target)
        if ret != None:
            break
        
    return ret

def find_sensor(src, name):
    curr = ""

def test_search(schema):
    print(search_schema(schema, "gateDriverBoardTemp")) 
    

def build_data_struct(subStructNames):
    masterData = {"data": {}}
    for name in subStructNames:
        print(name)
        masterData["data"][name] = {"type":apply_naming_convention(name), "limits":{}}
    print(masterData)
    return build_struct("data", masterData["data"])
    
def build_data_h(dataHStr):
    with open(DATA_HEADER_FILENAME, "w") as f:
        f.write(dataHStr)        
    return

def apply_naming_convention(name):
    return str(name) + "_t"

def build_struct(title, dataMembers):
    title = apply_naming_convention(title)
    structString = "typedef struct {} ".format(title)
    structString += "{\n"
    for key, val in dataMembers.items():
        structString += "\t{} {};\n".format(val["type"], str(key))
    structString += "}"
    structString += " {};\n\n\n".format(title)
    for key, val in dataMembers.items():
        structString += build_limits(key, val["limits"])
    return structString


def build_header():
    deps = ["stdint.h", "stdbool.h", "time.h"]
    header = WARNING_MSG
    for dep in deps:
        header += "#include <{}>\n".format(dep)
    header += "\n\n"
    return header

def slap_a_comment_on_that_bad_boy():
    return "/***\n*\n* Describe this struct here\n*\n***/\n\n"

def build_limits(sensorName, limits):
    limitStr = "/***\n*Limits for the state machine\n***/\n"
    for name,data in limits.items():
        limitStr += "#define {}_{}_MIN {}\n".format(sensorName.upper(),
                name.upper(), data["min"])
        limitStr += "#define {}_{}_MAX {}\n".format(sensorName.upper(),
                name.upper(), data["max"])
    limitStr += "\n\n"
    return limitStr

def main():
    schema = load_schema()
    dataHStr = ""
    defines = ""
    includes = ""
    structs = ""

    includes += build_header() 
    print(schema.keys())
    structs += build_data_struct(schema.keys())
    for key, val in schema.items():
        structs += slap_a_comment_on_that_bad_boy()
        structs += build_struct(key, val)

    # List in the order you want it added to the final file
    dataHStr += includes
    dataHStr += structs
    build_data_h(dataHStr)
#    print(dataHStr)
    return
    
def test():
    schema = load_schema()
    test_search(schema)

if __name__ == "__main__":
    #test()
    main()