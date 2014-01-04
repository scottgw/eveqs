note
	description : "chameneos application root class"
	date        : "$Date$"
	revision    : "$Revision$"

class
	APPLICATION

inherit
	ARGUMENTS

create
	make

feature {NONE} -- Initialization
	num_passes: INTEGER
	workers: ARRAYED_LIST [separate CHAMENEOS]
			
	
	make
		local
			chms1: ARRAYED_LIST [INTEGER]
			chms2: ARRAYED_LIST [INTEGER]
			n: Integer
		do
			n := argument(1).to_integer
			create workers.make (10)
			
			-- n := 600000
			create chms1.make(3)
			chms1.extend(0)
			chms1.extend(1)
			chms1.extend(2)
			run (n, chms1)
			print("Finished first%N")

			create chms2.make(10)
			chms2.extend(0)
			chms2.extend(1)
			chms2.extend(2)
			chms2.extend(1)
			chms2.extend(2)
			chms2.extend(0)
			chms2.extend(1)
			chms2.extend(2)
			chms2.extend(1)
			chms2.extend(0)
			run (n, chms2)
			print("Finished second%N")
		end

	run(n: INTEGER; chms: ARRAYED_LIST [INTEGER])
		local
			signal: separate SIGNAL
			worker: separate CHAMENEOS
			broker: separate BROKER
			-- workers: ARRAYED_LIST [separate CHAMENEOS]
			i: INTEGER
		do
			create signal.make()
			create broker.make(n)
			-- create workers.make(chms.count)

			from i := 1
			until i > chms.count
			loop
				create worker.make(chms[i], broker, signal, n)
				worker_request_meeting (worker)
				workers.extend(worker)
				i := i + 1
			end

			-- from i := 0
			-- until i >= chms.count
			-- loop
			--   shutdown workers.item(i)
			--   i := i + 1
			-- end
			-- shutdown signal
			-- shutdown broker
			-- signal_wait (signal)
		end

	worker_request_meeting (a_worker: separate CHAMENEOS)
		do
			a_worker.request_meeting
		end

	signal_wait (a_signal: separate SIGNAL)
		require
			a_signal.done
		do
		end
	
end
