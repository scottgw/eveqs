class THREADRING_WORKER

create make

feature

	has_token: BOOLEAN
	token: INTEGER
	next: separate THREADRING_WORKER
	id: INTEGER

	make (a_id: INTEGER; a_next: separate THREADRING_WORKER)
		do
			has_token := False
			id := a_id
			set_next (a_next)
		end

	set_next (a_next: detachable separate THREADRING_WORKER)
		do
			if attached a_next then
				next := a_next
			end
		end

	take_token(): INTEGER
		do
			has_token := False
			Result := token
		end

	pass (a_token: INTEGER)
		do
			token := a_token
			has_token := True
		end

	run()
		do
			-- print (id.out + " running%N")
			run_next (next)
		end

	run_next (a_next: separate THREADRING_WORKER)
		require
			a_next.has_token
		do
			-- print (id.out + " running next%N")
			token := a_next.take_token()
			-- print (id.out + " got token " + token.out + " from " + a_next.id.out + "%N")
			if token = 0 then
            print(id)
            print("%N")
				(create {EXCEPTIONS}).die(0)
			end
			token := token - 1

			has_token := True

			-- print (id.out + " setting next to run again%N")
			a_next.run()
			-- print (id.out + " end running next%N")
		end

end


