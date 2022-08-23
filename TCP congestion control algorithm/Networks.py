import math
import os
import sys
import argparse
import random
import matplotlib.pyplot as plt
from scipy.stats import bernoulli


def ReadArgments():
	parser = argparse.ArgumentParser()

	# ./mytcp -i <double> -m <double> -n <double> -f <double> -s <double> -T <int> -o outfile

	# optional arguments
	parser.add_argument("-i", "--ki", type=float, nargs='?', default=1, const=1)
	parser.add_argument("-m", "--km", type=float, nargs='?', default=1, const=1)
	parser.add_argument("-n", "--kn", type=float, nargs='?', default=1, const=1)

	# mandatory arguments
	parser.add_argument("-f", "--kf", type=float, nargs='?', default = None, const=None)
	parser.add_argument("-s", "--ps", type=float, nargs='?', default = None, const=None)
	parser.add_argument("-T", "--total", type=int, nargs='?', default = None, const=None)
	parser.add_argument("-o","--outfile",type = str, nargs='?', default = None, const=None)

	args = parser.parse_args()

	Ki = args.ki	# Intial exp phase will be quicker
	Km = args.km  # Steeper will be the exp phases
	Kn = args.kn # Slope of linear phase willbe high
	Kf = args.kf # Drop at time out will behigher
	Ps = args.ps # Less chance of getting timeout so less rounds to send T segments
	T = args.total
	outfile = args.outfile

	if(Ki == None or Km == None or Kn == None or Ps == None or T == None or Kf==None or outfile == None):
		print("Insufficient Arguments")
		exit()

	return Ki, Km, Kn, Kf, Ps, T,outfile


def SimulateTCP(Ki, Km, Kn, Kf, Ps, T,outfile):

	RWS = 1024 	# 1 MB
	MSS = 1 	# 1 KB
	threshold = RWS/2
	segments_sent = 0
	currentCW  = Ki*MSS
	cwValues = [currentCW]
	rounds = [1]
	numberOfRoundsCount = 2
	updateNumber = []
	cwUpdate = [currentCW]
	for i in range(T+1):
		updateNumber.append(i)

	
	with open(f"{outfile}.txt","a") as f:

		f.write(f"{currentCW}\n")
		linear = 0
		
		while(segments_sent < T):
			segementsToSend = min(math.ceil(currentCW),T-segments_sent)

			for i in range(segementsToSend):
				r = bernoulli.rvs(Ps, size=1)[0]

				if r == 1 :
					if linear:
						currentCW = min(currentCW + (Kn*(MSS*MSS)/currentCW),RWS)
						f.write(f"{currentCW}\n")
						cwUpdate.append(currentCW)
					else:
						currentCW = min(currentCW + (Kn*MSS),RWS)
						if (currentCW >= threshold):
							currentCW = threshold
							linear = 1
						f.write(f"{currentCW}\n")
						cwUpdate.append(currentCW)
				else:
					threshold = max(1,currentCW/2)
					currentCW = max(1,Kf*currentCW)
					linear = 0
					f.write(f"{currentCW}\n")
					cwUpdate.append(currentCW)

			segments_sent += segementsToSend
			cwValues.append(currentCW)
			rounds.append(numberOfRoundsCount)
			numberOfRoundsCount += 1


	return cwValues,rounds,cwUpdate,updateNumber


def main():

	# Read and Check input arguments
	Ki, Km, Kn, Kf, Ps, T,outfile = ReadArgments()

	if Ki < 1 or Ki > 4:
		sys.exit("Invalid value of Ki")
	if Km < 0.5 or Km > 2:
		sys.exit("Invalid value of Km")
	if Kn < 0.5 or Kn > 2:
		sys.exit("Invalid value of Kn")	
	if Kf < 0.1 or Kf > 0.5:
		sys.exit("Invalid value of Kf")	
	if Ps < 0 or Ps > 1:
		sys.exit("Invalid value of Ps")


	cwValues, Rounds,cwUpdate,updateNumber = SimulateTCP(Ki, Km, Kn, Kf, Ps, T,outfile)
	plt.plot(updateNumber,cwUpdate,marker = 'o')
	plt.xlabel("Update Number")
	plt.ylabel("Congestion window ")
	plt.title("Graph with Ki = "+ str(Ki) + ",Km = " + str(Km) + ",Kn = " + str(Kn) + ",Kf = "+str(Kf)+ ",Ps = " + str(Ps) + ",T = "+str(T))	
	temp = outfile + "_actual"
	plt.savefig(f"{temp}.jpg")
	plt.close()
	plt.plot(Rounds, cwValues , marker='o')
	plt.xlabel("Number of Rounds")
	plt.ylabel("Congestion window ")
	plt.title("Graph with Ki = "+ str(Ki) + ",Km = " + str(Km) + ",Kn = " + str(Kn) + ",Kf = "+str(Kf)+ ",Ps = " + str(Ps) + ",T = "+str(T))	
	temp = outfile + "_rounds"
	plt.savefig(f"{temp}.jpg")

if __name__ == "__main__":
	main()
