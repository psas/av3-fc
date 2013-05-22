#!/usr/bin/python
#
# codeGen.py was written by Ron Astin (rastin71 - github) 
# 03/16/13 PSU Senior Capstone project (Team Elderberry).
# Sponsor client: Portland State Aerospace Society (PSAS) http://psas.pdx.edu/
#
# Team Elderberry:
#	Ron Astin
#	Chris Glasser
#	Jordan Hewitt
#	Mike Hooper
#	Josef Mihalits
#	Clark Wachsmuth

import sys
import re
import yaml
import copy
from os import path, access, R_OK
from collections import defaultdict

class ParserStates:
    # Since we have multiple MIML files now we need phases for processing.
    # Expansion allows handler functions that expect data in other files the opportunity
    #   to pull in that external data and place it in the parse tree.
    # Validation allows handler functions the opportunity to examine other data in the tree
    #   to ensure it is ready for use, including data pulled in by other functions.
    # Parsing is where the actual parsing work happens, where handler functions generate output.
    # Actual output writting happens when all these phases are complete and is not part of "parsing".
    # There is a 4th stage (not really a stage), purge. In which a ParserHandlers function is called to
    # commit last minute stuff to the OutputGenerator. For some output requirements it may be easier to 
    # stage to a local ParserHandler structure in Parse, then stage later.
    Expand, Validate, Parse = range(3)

class ErrorLogger:
    # Log errors or warnings here, then check periodically.
    # Code Generator uses no warnings, but they can be fun for debugging.
    # The check method exits if errors exist, but just prints out warnings and keeps going.
    def __init__(self):
        self.errors = []
        self.warnings = []

    def new_error(self, message):
        self.errors.append(message)

    def new_warning(self, message):
        self.warnings.append(message)

    def append_error(self, message):
        if len(self.errors) > 0: # append to empty list just adds new error.
            message = self.errors.pop() + message 
        self.errors.append(message)

    def append_warnings(self, message):
        if len(self.warnings) > 0: # append to empty list just adds new error.
            message = self.warnings.pop() + message 
        self.warnings.append(message)

    def has_errors(self):
        if len(self.errors) > 0:
            return True
        return False

    def has_warnings(self):
        if len(self.warnings) > 0:
            return True
        return False

    def check_file(self, filename):
        if path.isfile(filename) == False:
            self.new_error("File: '" + filename + "' cannot be found!")
        elif access(filename, R_OK) == False:
            self.new_error("File: '" + filename + "' cannot be opened for reading!")
        else:
            return True
        return False

    def check(self):
        if self.has_warnings():
            print (len(self.warnings), " warning(s) encountered!")
            for warning in self.warnings:
                print (warning)
        if self.has_errors():
            print (len(self.errors), " error(s) encountered!")
            for error in self.errors:
                print (error)
            sys.exit(0)


class OutputGenerator:
    # What we want here is:
    #    RootDictionary { ModeDictionary { LevelDictionary { OutputList [] }}}
    #
    # So OutputGen{Code}{1}{Include File1, Include File2}
    # or OutputGen{Header1}{1}{Function PrototypeA, Function PrototypeB}
    #
    # This way different Handlers can be invoked for different purposes 
    #   and order their output as they wish.
    # The constructor strucure mode_flages_files should coincide with the modes allowed.
    # So if you add to one add to the other. Same mode token, its used across them in the boolean run check!

    def __init__(self, mode_flags_files):
        self.output = defaultdict(lambda: defaultdict(list))
        self.mode_flags_files = mode_flags_files

    def append(self, mode, level, data):
        self.output[mode][level].append(data)

    def display(self):
        for mode in self.output.keys():
            if self.mode_flags_files[mode]['run'] == True:
                print (mode + ": " + self.mode_flags_files[mode]['file'])
                for level in sorted(self.output[mode].keys()):
                    for message in self.output[mode][level]:
                        print (mode, "->", level, "->", message)
            print ("\n") # separate modes
            
    def write_out(self):
        for mode in self.output.keys():
            if self.mode_flags_files[mode]['run'] == True:
                f = open(self.mode_flags_files[mode]['file'], "w")
                for level in sorted(self.output[mode].keys()):
                    for message in self.output[mode][level]:
                        f.write(message + '\n')

class Parser:

    def __init__(self, filename):
        self.errors = ErrorLogger()
        # declar modes_flags_files, the call to argument_check will set run by arg flags
        modes_flags_files = {'code': {'run': False, 'file': None}, 'make': {'run': False, 'file': None}, 'header': {'run': False, 'file': None}}

        # Check command line arguments used to invoke codeGen, if good check file.
        if self.argument_check(modes_flags_files):
            self.errors.check_file(filename)
        self.errors.check()
        # Then read config file.
        try:
            self.config = yaml.load(open(filename, 'r'))
        except Exception as e:
            self.errors.new_error("YAML parsing error: " + str(e))
        self.errors.check()

        # get filename configuration data
        modes_flags_files['code']['file'] = self.config['code_filename']
        modes_flags_files['header']['file'] = self.config['header_filename']
        modes_flags_files['make']['file'] = self.config['make_filename']
        # remove filename stuff from config, this makes it so only handler data is left. Better for handlers!
        del(self.config['code_filename'])
        del(self.config['header_filename'])
        del(self.config['make_filename'])
        # get allowed types configuration data
        allowed_types = self.config['allowed_types']
        del(self.config['allowed_types'])
        # Setup a ParserHandlers obj
        self.handler_functions = ParseHandlers(self, allowed_types)

        # Make paths lists for easier parsing
        for handler in self.config.keys():
            self.config[handler]['path'] = self.config[handler]['path'].split("/")
        self.path = ['']
        self.state = None
        self.output = OutputGenerator(modes_flags_files)

    def parse(self):
        # top level 'public' function. Since we have external MIML docs we need to pull those in
        # before we crawl, so order of processing matters even though order of MIML elements does not.
        try:
            self.master = yaml.load(open(self.miml_file, 'r'))
        except Exception as e:
            self.errors.new_error("YAML parsing error: " + str(e))
            self.errors.check()
        # Do Expand, Validate, Parse
        while self.transition() == True:
            self.crawl(self.master)
        # Output
        # Let's you see stuff, uncomment when writing MIML extensions and trying to
	#     figure out where to insert content in OutputGenerator.
        # self.output.display() 
        # Make files!!!
        self.output.write_out() 

    def crawl(self, data):
        # Recursive function "Weee!"
        # Different structure walking for dict/list/scalar
        # path works as stack of directories (push/pop)
        if type(data).__name__=='dict':
            for key in data.keys():
                self.path.append(key)
                if self.matchpath(data[key]) == False:
                    self.crawl (data[key])
                self.path.pop()
        elif type(data).__name__=='list':
            for element in data:
                self.path.append(element)
                if self.matchpath(element) == False:
                    self.crawl (element) 
                self.path.pop()           
        else:
            self.matchpath(data)

    def transition(self):
        # This function can be cleaned up a bit, its unfolded to let me figure out what should be same/different atm.
	    # check for errors thrown during last phase.
        self.errors.check()
        return_value = True
        if self.state == None:
            # Set new buffer, copy master to unhandled.
            self.buffer = {}
            self.unhandled = copy.copy(self.master)
            self.state = ParserStates.Expand
	    # Uncomment when adding MIML handlers that deal with Expansion.
	    # print ("Expand This:")
	    # print (yaml.dump(self.master))
        elif self.state == ParserStates.Expand:
            # Make buffer contents master. 
            self.master = self.buffer
            self.buffer = {}
	        # Necessary to check unhandled from Expand or we will miss problems during buffer transfer
            if not self.unhandled == {}:
                self.errors.new_error("Unhandled MIML content at end of Expand state! " + str(self.unhandled))
            self.unhandled = copy.copy(self.master)
            self.state = ParserStates.Validate
	    # Uncomment when adding MIML handlers that deal with Validation.
	    # print ("Validate This:")
	    # print (yaml.dump(self.master))
        elif self.state == ParserStates.Validate:
            if not self.unhandled == {}:
                self.errors.new_error("Unhandled MIML content at end of Validate state! " + str(self.unhandled))
            # Let's not do buffers and unhandled structs after this point, saves 'pointless' code in handlers.
            # Make buffer contents master. Should we allow buffering during Validate? Seems like a no.
            self.master = self.buffer
            self.state = ParserStates.Parse
            # Uncomment when adding MIML handlers that deal with Parsing.
	    # print ("Parse This:")
	    # print (yaml.dump(self.master))
        else:
            # purge staged data. Our 4th state, kinda...
            self.handler_functions.purge()
            # last transition so return false.
            return_value = False
	    # Check for errors thrown during transition
        self.errors.check()
        return return_value
        

    def matchpath(self, data):
        # This method returns True if a handler decides no other parsing is required for
        # the data it handles, for the mode it is in.
        # 
        # Goal: Flexible not complicated!!! We could support all kinds of crazy, but we won't!
        #    '*' means single position wildcard, not arbitrary number of elements.
        #    handler paths that do not begin with '/' should not contain any '/'.
        #      They represent single token matches for the immediate point.
        #
        # Possible Match Cases: self.path = a / self.config[key]['path'] = b
        # 1 - len(a) == len(b) and ( a[x] == b[x] or b[x] == '*' )
        # 2 - len(a) != len(b) && len(b) == 1
        #
        # Someone may put '*' for path, not sure if I want to make that illegal.        

        return_value = False
        for key in self.config.keys():
            if len(self.path) == len(self.config[key]['path']):
                # path == len of current handler path, so we are checking #1 above (comments)
                match = True
                for position in range(0, len(self.path)):
                    if (not self.path[position] == self.config[key]['path'][position] and
                            not self.config[key]['path'][position] == '*'):
                        match = False
                if match == True:
                    return_value = return_value | self.call_handler_function(key, data)
            if len(self.config[key]['path']) == 1 and self.config[key]['path'][0] == self.path[-1]:  
                # only len 1 for path and we matched last element. Note: /foo is length 2 path (invisible root)     
                return_value = return_value | self.call_handler_function(key, data)
        return return_value

    def call_handler_function(self, key, data):
        # NOTE: This is where the handler functions get called!!!
        if not type(data).__name__ == self.config[key]['type']:
            # type of data is not same as what was declared in cg.conf, so error.
            self.errors.new_error("Handler type mismatch. " + key + " expects " + self.config[key]['type'] + " received " + type(data).__name__)
            return False
        else:
            # call hander function 'key', in ParserHandlers, passing data 
            return getattr(self.handler_functions, key)(data)

    def argument_check(self, modes_flags_files):
        # checks command line arguments, configures modes based on flags.
        if len(sys.argv) > 3 or len(sys.argv) < 2:
            self.errors.new_error("Illegal number of arguments! Expected 1 or 2, received: "
            + str(len(sys.argv) -1))
            return False
        self.miml_file = sys.argv[len(sys.argv) - 1]
        mode_flags = "-cmh" if len(sys.argv) == 2 else sys.argv[1]
        # Pretty good checking, someone can put in multiple valid letter flags and it will work as if 1 was supplied.
        # Not sure why this would happen (typo?) but its "valid" by way of this code.
        match = re.match(r"^-[cmh]+$", mode_flags)
        if match:
            if re.match(r"(.*c)", mode_flags):
                modes_flags_files['code']['run'] = True
            if re.match(r"(.*m)", mode_flags):
                modes_flags_files['make']['run'] = True
            if re.match(r"(.*h)", mode_flags):
                modes_flags_files['header']['run'] = True
            return True
        else:
            self.errors.new_error("Illegal mode usage, expecting -[chm]. Given : " + mode_flags)
        return False

class ParseHandlers:

    def __init__(self, parser, allowed_types):
        self.parser = parser
        # to support validate_params
        self.allowed_types = allowed_types
        # objects for single line make file
        self.objects = []

    def purge(self):
        # Required function, not part of config-based handlers
        # Called after Parsing phase, allows handlers to stage data and then commit to OutputGenerator after parse stage.
        # or allows single time setup data, like fcfutils.h include, or carriage returns for pretty output.
        o = self.parser.output
        o.append("code", 1, "#include \"fcfutils.h\"")
        o.append("code", 6, "\n")
        o.append("code", 11, "\n")
        o.append("code", 16, "\n")
        o.append("make", 6, "\n")
        if len(self.objects) > 0:
            self.parser.output.append("make", 5, "OBJECTS += " + ' '.join(self.objects))

    def parse_sources(self, data):
        p = self.parser
        e = p.errors
        o = p.output
        if p.state == ParserStates.Expand:
            # Pull in external file data, place in buffer
            del(p.unhandled['sources'])
            p.buffer['modules'] = {}
            p.buffer['source_order'] = data
            for source in data:             
                if (e.check_file(source[1])):
                    try:
                        p.buffer['modules'][source[0]] = yaml.load(open(source[1], 'r'))
                    except Exception as e:
                        e.new_error("YAML parsing error: " + str(e))
            return True
        elif p.state == ParserStates.Parse:
            module_miml = []
            for source in data:
                module_miml.append(source[1])
            o.append("make", 10, o.mode_flags_files['code']['file'] + " " + o.mode_flags_files['header']['file'] + ": " + p.miml_file + " " + ' '.join(module_miml))
            o.append("make", 10, "\t./codeGen.py -ch " + p.miml_file)
        return True # Nothing responds to data under here, left in so includes/final can figure out what order to stage data.

    def parse_messages(self, data):
        p = self.parser
        e = p.errors
        o = p.output
        if p.state == ParserStates.Expand:
            # Nothing to expand, but buffer messages for later passes.
            del(p.unhandled['messages'])
            p.buffer['messages'] = data
            return True
        elif p.state == ParserStates.Validate:
            for message in data.keys():
                sender = message.split('.')
                if not len(sender) == 2:
                    e.new_error("Illegal Sender syntax: " + message)
                elif not sender[0] in p.master['modules']:
                    e.new_error("Sending source " + sender[0] + " not loaded as module.")
                elif not sender[1] in p.master['modules'][sender[0]]['senders']:
                    e.new_error("Sending message " + sender[1] + " not defined as sender for " + sender[0])
                else:
                    sender_params = p.master['modules'][sender[0]]['senders'][sender[1]]
                    for rec in data[message]:
                        receiver = rec.split('.')
                        if not len(sender) == 2:
                            e.new_error("Illegal Receiver syntax: " + rec + " for message " + message)
                        elif not receiver[0] in p.master['modules']:
                            e.new_error("Receiver: " + receiver[0] + " not loaded as module.")
                        elif not receiver[1] in p.master['modules'][receiver[0]]['receivers']:
                            e.new_error("Receiver function " + receiver[1] + " not defined as receiver for " + receiver[0])
                        elif not len(sender_params) == len(p.master['modules'][receiver[0]]['receivers'][receiver[1]]):
                            e.new_error("Message " + str(sender) + " cannot send to receiver " + str(rec) +
                            ". Number of arguments must be the same in both functions.")
                        else:
                            pos = 0
                            for param in sender_params:
                                if not param[1] == p.master['modules'][receiver[0]]['receivers'][receiver[1]][pos][1]:
                                    e.new_error("Message " + message + " cannot send to receiver " + 
                                    rec + ". Type mismatch on argument " + str(pos + 1))
                                pos += 1
            del(p.unhandled['messages'])
            p.buffer['messages'] = data
            return True
        elif p.state == ParserStates.Parse:
            for message in data.keys(): # for each message
                (src, func) = message.split('.')
                args = []
                params = []
                types = []
                for caller_param in p.master['modules'][src]['senders'][func]: # for each param in caller
                    args.append(caller_param[1] + " " + caller_param[0])
                    params.append(caller_param[0])
                    types.append(caller_param[1])
                o.append("header", 10, "void " + func + "(" + ', '.join(types) + ');')
                o.append("code", 20, "void " + func + "(" + ', '.join(args) + ') {')                    
                for receivers in data[message]: # for each receiver
                    (rsrc, rfunc) = receivers.split('.')
                    o.append("code", 20, "    " + rfunc + "(" + ', '.join(params) + ');')
                o.append("code", 20, "}\n")
            return True
        return False

    def parse_modules(self, data):
        p = self.parser
        e = p.errors
        o = p.output
        # Must return False or other module matches will not happen.
        # No need for Expansion code, modules are created during source expansion.
        if p.state == ParserStates.Validate:
            for source in data.keys():
                for key in data[source]:
                    if not key in ('include', 'object', 'init', 'final', 'senders', 'receivers'):
                        e.new_error("Module: " + source + " contains illegal component: " + key)
            del(p.unhandled['modules'])
        p.buffer['modules'] = data
        return False

    def parse_includes(self, data):
        # handles include files.
        p = self.parser
        e = p.errors
        o = p.output
        if p.state == ParserStates.Validate:
            if not re.match(r"\w+\.h", data):
                e.new_error("Illegal header file format: " + data + " in " + '/'.join(p.path))
        elif p.state == ParserStates.Parse:
            o.append("code", 5, "#include \"" + data + "\"")
        return True

    def parse_objects(self, data):
        # handles object files for the make file, needs to add 1 row to make file so stages
        # into (self.objects), purge makes output.
        p = self.parser
        e = p.errors
        if p.state == ParserStates.Validate:
            if not re.match(r"\w+\.o", data):
                e.new_error("Illegal object file format: " + data + " in " + '/'.join(p.path))
        elif p.state == ParserStates.Parse:
            self.objects.append(data)
        return True

    def validate_inits(self, data):
        # validates a modules initialize functions, output is generated via the module handler.
        p = self.parser
        e = p.errors
        if p.state == ParserStates.Validate:
            if not re.match(r"\w+\([^)]*\);", data):
                e.new_error("Illegal initialize function: " + data + " in " + '/'.join(p.path))
        return True

    def parse_init_final(self, data):
        p = self.parser
        o = p.output
        e = p.errors
        if p.state == ParserStates.Validate:
            del(p.unhandled['source_order'])
            p.buffer['source_order'] = data
        elif p.state == ParserStates.Parse:
            finals = []
            o.append("code", 10, "void fcf_initialize() {")
            for source in data:
                token = source[0]
                if 'init' in p.master['modules'][token]:
                    o.append("code", 10, "    " + p.master['modules'][token]['init'])
                if "final" in p.master['modules'][token]:
                    finals.append(p.master['modules'][token]['final'])
            o.append("code", 10, "}")
            o.append("code", 15, "void fcf_finalize() {")
            while len(finals) > 0:
                o.append("code", 15, "    " + finals.pop())  
            o.append("code", 15, "}")                 
        return True
    
    def validate_finals(self, data):
        # validates a modules finalize functions, output is generated via the module handler.
        p = self.parser
        e = p.errors
        if p.state == ParserStates.Validate:
            if not re.match(r"\w+\([^)]*\);", data):
                e.new_error("Illegal finalize function: " + data + " in " + '/'.join(p.path))
        return True

    def validate_senders(self, data):
        # validate_params wrapper that targets senders
        return self.validate_params(data)

    def validate_receivers(self, data):
        # validate_params wrapper that targets receivers
        return self.validate_params(data)

    def validate_params(self, data):
        # Validate sender and receiver parameters, checks that each parameter has 2 elements
        # and that the second is an approved type (self.allowed_types)
        p = self.parser
        e = p.errors
        if p.state == ParserStates.Validate:
            for param in data:
                if not len(param) == 2:
                    e.new_error("Illegal parameter definition: " + str(param) + " in " + '/'.join(p.path))
                datatype = re.match(r"(?:const\s)?((?:unsigned\s)?\w+)(?:\s?[*&])?", param[1]).group(1)
                if not datatype in self.allowed_types:
                    e.new_error("Illegal parameter type: " + str(param[1]) + " in " + '/'.join(p.path))
        return True
            
parser = Parser('./cg.conf')
parser.parse()
