import re, sys

find_t = re.compile("= (\\d+)")

with open("gdb.output") as gdbout:
	lines = gdbout.read().split("\n")

thread1_times = list(map(lambda s: find_t.search(s).group(1),list(filter(lambda s: "Thread 1" in s, lines))))
thread2_times = list(map(lambda s: find_t.search(s).group(1),list(filter(lambda s: "Thread 2" in s, lines))))

intervals1 = [ int(thread1_times[i+1]) - int(thread1_times[i]) for i in range(len(thread1_times)-1) ]
intervals2 = [ int(thread2_times[i+1]) - int(thread2_times[i]) for i in range(len(thread2_times)-1) ]

print intervals1
print intervals2

tolerance = 2

target1 = 500
target2 = 1000

fails = 0

for i in intervals1:
	if i > target1-tolerance and i < target1+tolerance:
		print("{} < {} < {} -> PASS".format(target1-tolerance , i , target1+tolerance ))
	else:
		print("{} < {} < {} -> FAIL".format(target1-tolerance , i , target1+tolerance ))
		fails+=1

for i in intervals2:
	if i > target2-tolerance and i < target2+tolerance:
		print("{} < {} < {} -> PASS".format(target2-tolerance , i , target2+tolerance ))
	else:
		print("{} < {} < {} -> FAIL".format(target2-tolerance , i , target2+tolerance ))
		fails+=1

print("{} fails {}".format(fails   , "check this testcase!"if fails > 0 else ""))

if fails > 0:
	sys.exit( -1)
else:
	sys.exit(0)

