.PHONY: fork thread select poll epoll

fork:
	gcc Client.c -o CC -lpthread
	gcc forkServer.c -o fork -lpthread
	sleep 1
	xterm -geometry 150x32 -T "Server" -n "Server" -bg black -fg yellow -e bash -c './fork; exec bash' &
	sleep 1
	xterm -geometry 150x32 -T "Client" -n "Client" -bg black -fg yellow -e bash -c './CC; exec bash' &

thread:
	gcc Client.c -o CC -lpthread
	gcc pthreadServer.c -o thread -lpthread
	sleep 1
	xterm -geometry 150x32 -T "Server" -n "Server" -bg black -fg yellow -e bash -c './thread; exec bash' &
	sleep 1
	xterm -geometry 150x32 -T "Client" -n "Client" -bg black -fg yellow -e bash -c './CC; exec bash' &

select:
	gcc Client.c -o CC -lpthread
	gcc selectServer.c -o select -lpthread
	sleep 1
	xterm -geometry 150x32 -T "Server" -n "Server" -bg black -fg yellow -e bash -c './select; exec bash' &
	sleep 1
	xterm -geometry 150x32 -T "Client" -n "Client" -bg black -fg yellow -e bash -c './CC; exec bash' &

poll:
	gcc Client.c -o CC -lpthread
	gcc pollServer.c -o poll -lpthread
	sleep 1
	xterm -geometry 150x32 -T "Server" -n "Server" -bg black -fg yellow -e bash -c './poll; exec bash' &
	sleep 1
	xterm -geometry 150x32 -T "Client" -n "Client" -bg black -fg yellow -e bash -c './CC; exec bash' &

epoll:
	gcc Client.c -o CC -lpthread
	gcc ePollServer.c -o epoll -lpthread
	sleep 1
	xterm -geometry 150x32 -T "Server" -n "Server" -bg black -fg yellow -e bash -c './epoll; exec bash' &
	sleep 1
	xterm -geometry 150x32 -T "Client" -n "Client" -bg black -fg yellow -e bash -c './CC; exec bash' &
