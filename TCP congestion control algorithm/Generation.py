import os
import matplotlib.pyplot as plt

ki = [1,4]
km = [1,1.5]
kn = [0.5,1]
kf = [0.1,0.3]
ps = [0.99,0.9999]
count  = 1

for i in ki:
	for j in km:
		for k in kn:
			for l in kf:
				for m in ps:
					os.system(f"python3 Networks.py -i {i} -m {j} -n  {k} -f {l} -s {m} -T 1000 -o fig{count}")
					count += 1