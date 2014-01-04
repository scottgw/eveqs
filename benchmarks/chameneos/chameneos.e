class CHAMENEOS

create make

feature
	c: Integer
	broker: separate BROKER
	signal: separate SIGNAL
	max: Integer

	make(c_: Integer; broker_: separate BROKER; signal_: separate SIGNAL; max_: Integer)
		do
			c := c_
			broker := broker_
			signal := signal_
			max := max_
		end

	compl(c1: Integer; c2: Integer): Integer
		do
			if c1 = 0 then
				if c2 = 0 then
					Result := 0
				elseif c2 = 1 then
					Result := 2
				else
					Result := 1
				end
			elseif c1 = 1 then
				if c2 = 0 then
					Result := 2
				elseif c2 = 1 then
					Result := 1
				else
					Result := 0
				end
			else
				if c2 = 0 then
					Result := 1
				elseif c2 = 1 then
					Result := 0
				else
					Result := 2
				end
			end
		end

	request_meeting()
		local
			met: Boolean
		do
			from met := True
			until not met
			loop
				met := contact_broker(broker)
				if met then
					meet_other (other_cham)
					-- c := compl(c, a_other_c)
					-- if n >= max then
					-- 	send_signal (signal)
					-- end
					meet_with (other_c, n, False)
				end
			end
      end

	-- These are basically just to communicate between `contact_broker'
	-- and `request_meeting'
	other_cham: separate CHAMENEOS
	other_c: Integer
	n: Integer
	
	meet_other(a_other_cham: separate CHAMENEOS)
		do
			a_other_cham.meet_with(c, n, True)
		end

	contact_broker(a_broker: separate BROKER): BOOLEAN
		do
			Result := a_broker.register_cham(c, Current)
			if Result then
				other_cham := a_broker.current_cham
				other_c := a_broker.current_c
				n := a_broker.n
				a_broker.clear().do_nothing()
			end
		end	 
	 
	meet_with(a_other_c: Integer; a_n: Integer; restart: Boolean)
		do
			c := compl(c, a_other_c)
			if a_n >= max then
				send_signal (signal)
			elseif restart then
				request_meeting()
			end
		end

	send_signal(a_signal: separate SIGNAL)
		do
			a_signal.signal()
		end
	 
end
