with open("sources.list","r") as f, open("../sources.list","w") as w:
	lines = f.readlines()
	for line in lines:
		line = line.strip()
		if line[-5:]!=".java":
			continue
		else:
			location = -1
			for i in range(len(line)):
				if line[i]=='/':
					location = i+1
					break
			w.write("./src" + line[location:] + "\n")
			 
