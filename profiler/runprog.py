#!/usr/bin/env python
#import logger
import logging, sys, os, time, shutil, pexpect, subprocess, re, csv

csv_target = "profiler-data/output.csv"
SIG_PROF_KILL = 2929
SIGQUIT = 3
SIGINT = 2
STDOUT = 1
PIPE_FD = 3


headerlabels = ",".join(["Time run", "Function name", "Percentage of time",
                             "Cumulative seconds", "Self seconds",
                             "Number of calls", "Seconds per call (self)",
                             "Seconds per call (total)"])

def print_usage():
    print(
        """
Usage: ./runprog time program_name [program_arguments ...] [[-c|--csv]="file"]

time                The amount of time to run the program. Use the format
                    HH:MM:SS where "HH" is hours, "MM" is minutes, and "SS"
                    is seconds. For eample, five hours, six seconds will be
                    written as "05:00:06"

program-name        The name of the program to run

program_arguments   Arguments to supply to the program program_name.

-c|--csv="file"     The filename to use for CSV output.
"""
    )



class CFunc:

    def __init__(self, gprof_list=None):
        if (gprof_list != None):
            try:
                self.set_func(str(gprof_list[0]),
                              str(gprof_list[1]),
                              str(gprof_list[2]),
                              str(gprof_list[3]),
                              str(gprof_list[4]),
                              str(gprof_list[5]),
                              str(gprof_list[6]))
            except:
                logging.warn("List %s does not have enough elements."
                % (str(gprof_list)))

    def set_func(self, perc_time, cumu_sec, self_sec, calls, self_sec_per_call,
                 tot_sec_per_call, name):

        self.perc_time = perc_time
        self.cumu_sec = cumu_sec
        self.self_sec = self_sec
        self.calls = calls
        self.self_sec_per_call = self_sec_per_call
        self.tot_sec_per_call = tot_sec_per_call
        self.name = name

    def __str__(self):
        return"""
===== Function: %s ======
Percentage of time:         %s
Cumulative seconds:         %s
Self seconds:               %s
Number of calls:            %s
Seconds per call (self):    %s
Seconds per call (total):   %s
        """ % (str(self.name),
               str(self.perc_time),
               str(self.cumu_sec),
               str(self.self_sec),
               str(self.calls),
               str(self.self_sec_per_call),
               str(self.tot_sec_per_call))

    def tocsv(self):
        return ",".join([
                  self.name,
                  str(self.perc_time),
                  str(self.cumu_sec),
                  str(self.self_sec),
                  str(self.calls),
                  str(self.self_sec_per_call),
                  str(self.tot_sec_per_call)])

    def as_list(self):
        return [
                  self.name,
                  str(self.perc_time),
                  str(self.cumu_sec),
                  str(self.self_sec),
                  str(self.calls),
                  str(self.self_sec_per_call),
                  str(self.tot_sec_per_call)]

class GProfOut:
    def __init__(self):
        self.time_run = 0
        self.functions = {}

    def __str__(self):
        s = "\n"
        for (name, f) in self.functions.iteritems():
            s += "%s" % f
            s += ">> Run time for %s: %s seconds" % (name, self.time_run)
        return s

    def tocsv(self):
        mycsvs = []
        for (name,func) in self.functions.iteritems():
            mycsvs.append("%s,%s" % (self.time_run, func.tocsv()))
        return '\n'.join(mycsvs)

    def as_list(self):
        lst = []
        for func in self.functions.itervalues():
            lst.append([str(self.time_run)] + func.as_list())
        return lst

##
# Run the program for the specified number of times.
# @param t: how long to run the program.
# @param args: The program arguments.
#
def run_for_time(t, args):
    pid = os.fork()
    outfd = 0
    sleep_time = t
    if (pid == 0):   # We're the child
        logging.info("The child prcess is %d" % (os.getpid()))
        logging.info("Will sleep for %d seconds" % (sleep_time))
        os.dup2(outfd, STDOUT)
        os.execvp(args[0], args)
    else:           # We're the parent.
        time.sleep(sleep_time)

        # Send the SIGINT signal
        os.kill(pid, SIGINT)

        os.dup2(STDOUT, outfd)

#
# Human-readable utility function for declaring which test
# is running
# @param list arg_time A list consisting of
#   <tt>(hours, minutes, seconds)</tt>
# @param int iter The iteration number.
def print_test_header(arg_time, iter):
    t = secs_to_arr(arg_time)
    init_header = "## TEST (%d) - Time %s h, %s m, %s s ##" % (iter,
        t[0], t[1], t[2])
    border = "#" * len(init_header)

    logging.info(border)
    logging.info(init_header)
    logging.info(border)


##
# Given a filename in gprof format, parse its information
# for analysis.
# @param filename: The filename in gprof format.
# @return A GProfOut object representing the information.
def parse_output_file(filename):
    line_regex = '((\d)?\s*\d+\.?\d+)+\s+\w*'
    split_regex = '(\s){1,7}'

    re_line = re.compile(line_regex)
    re_split = re.compile(split_regex)

    p = subprocess.Popen("gprof -b --flat-profile %s %s" %
                    (' '.join(sys.argv[2:]),filename), shell=True,
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    lines = []
    line = ""
    while (True):
        line = p.stdout.readline()
        if re.match(re_line, line) != None:
            lines.append(line)
        if line == "" and p.poll() != None:
            break

    print("\n")
    funclist= []
    for l in lines:
        lines_list = re.split(re_split, l)
        lines_list2 = []
        for ll1 in lines_list:
            if ll1 not in ['', ' ', '  ', '\n', '\t']:
                lines_list2.append(ll1)
        funclist.append(lines_list2)

    gpo = GProfOut()
    for func in funclist:
        if len(func) != 7:
            func = fill_func(func)
        print func
        gpo.functions[func[6]] = CFunc(func)

    return gpo

##
# Do an educated guess as to which elements are missing
# And fill them in with null strings.
# @param func_def: The function definiton (in list format) to fill out.
# @return A new function definition in list format.
def fill_func(func_def):
    logging.debug("Fixing list: %s" % (func_def))
    real_last_i = 7
    last_i = len(func_def) - 1
    diff = real_last_i - last_i
    logging.debug("Moving element %d to %d" % last_i, real_last_i)
    func_def.extend([""]*(diff-2) + [func_def[last_i]])
    logging.debug("Fixed list: %s" % (func_def))
    return func_def


def arr_to_secs(time):
    return (int(time[0]) * 3600) + (int(time[1]) * 60) + int(time[2])

def secs_to_arr(time):
    return [
        (time / 3600) % 3600,
        (time / 60 ) % 60,
        (time % 60) % 60
    ]

##
# Dump the resulting gprof output to a csv file.
# @param results: The results parsed from gprof output -- as a list.
# @param header: The header for the CSV file.
def dump_to_csv_file(results, header):
    #  Try to open the file. Create it if it doesn't exist.
    logging.debug("Printing results: %s" % (results))
    write_header = False
    target_dir = csv_target.split("/")[0]
    logging.debug("Does %s exist?" % target_dir)
    if not os.path.exists(target_dir):
        logging.debug("Unfortunately, no.")
        subprocess.call(["mkdir", target_dir])
        #os.execl("/bin/mkdir", "mkdir", "profiler-data")

    logging.debug("Does %s exist?" % csv_target)
    if not os.path.exists(csv_target):
        #os.execl("/bin/touch", "touch", csv_target)
        subprocess.call(["touch", csv_target])
        write_header = True

    csv_file = open(csv_target, "r+b")

    lines = csv_file.readlines()
    if len(lines) > 0:
        if lines[0] != header:
            csv_file.close()
            csv_file = open(csv_target, "a+b")
            csv_file.write(header)
            csv_file.writelines(lines)
            csv_file.close

    with open(csv_target, "ab") as csv_file:
        target = csv.writer(csv_file)
        if (write_header):
            target.writerow(header)
        for res in results:
            target.writerow(res)

def main():

    # Check that the user has valid arguments.
    if (len(sys.argv) < 3):
        logging.error("Invalid usage.")
        print_usage()
        sys.exit()

    # Store the arguments in variables (just for convenience)
    arg_time = sys.argv[1]
    arg_args = sys.argv[2:]


    # Generate a new gmon.out file, bumping up the last file index.

    # Set up variables and time array for testing.
    loop = 0
    times = []
    for t in arg_time.split(","):
        tim = arr_to_secs(t.split(":"))
        times.append(tim)
    print (times)


    # Assign a dictionary where gmon file name => time
    for ta in times:
        loop += 1
        print_test_header(ta, loop)
        run_for_time(ta, arg_args)
        gout = parse_output_file("gmon.out")
        gout.time_run = ta
        dump_to_csv_file(gout.as_list(), headerlabels)

    sys.exit()

if __name__=="__main__":
    logging.basicConfig(
        format="%(levelname)s\t[%(asctime)s]\t%(msg)s",
        level=logging.DEBUG)
    main()
