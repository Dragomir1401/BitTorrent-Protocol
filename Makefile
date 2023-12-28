build:
	mpicxx -o tema3 peer.cpp thread_funcs.cpp tracker.cpp peer_info.cpp input_parsers.cpp tracker_info.cpp swarm_info.cpp main.cpp -Wall

clean:
	rm -rf tema3
