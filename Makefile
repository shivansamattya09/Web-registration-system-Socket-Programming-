all: serverM.cpp serverC.cpp serverCS.cpp serverEE.cpp client.cpp

	g++ -o serverM serverM.cpp
	g++ -o serverC serverC.cpp
	g++ -o serverEE serverEE.cpp
	g++ -o serverCS serverCS.cpp
	g++ -o client client.cpp

.hello: serverM
serverM:
		./serverM

.hello: serverC
serverC:
		./serverC

.hello: serverCS
serverCS:
		./serverCS

.hello: serverEE
serverEE:
		./serverEE

.hello: client
client:
		./client
clean:
	 rm serverM
	 rm client
	 rm serverC
	 rm serverCS
	 rm serverEE
