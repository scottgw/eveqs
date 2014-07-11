note
	description: "Summary description for {COND_WORKER}."
	author: ""
	date: "$Date$"
	revision: "$Revision$"

class
	COND_WORKER

create
	make

feature
	num_iterations: INTEGER

	-- Each worker shares a separate VAR.
	make (v: separate VAR; max: INTEGER)
		do
			var := v
      num_iterations := max
		end

	var: separate VAR

	-- Producers wait for the number in var to be odd many times.
	produce
		local
			i: INTEGER
		do
			from i := 1
			until i > num_iterations
			loop
        -- print ("produce odd " + i.out + "%N")
        produce_odd (var)
				i := i + 1
			end
      done := True
		end

	produce_odd (v: separate VAR)
		require
			v.i \\ 2 = 1
    local
      i: INTEGER
		do
			-- -- print ("Odd: " + v.i.out + "%N")
      -- print ("odds after wait%N")
      i := v.i
      -- print ("odds after fetch%N")
      v.set_i (v.i + 1)
		end

	-- Consumers wait for the number in var to be even many times.
	consume
		local
			i: INTEGER
		do
			from i := 1
			until i > num_iterations
			loop
        -- print ("produce even " + i.out + "%N")
				consume_even (var)
				i := i + 1
			end
      done := True
		end

	consume_even (v: separate VAR)
		require
			v.i \\ 2 = 0
    local
      i: INTEGER
		do
      -- -- print ("Even: " + v.i.out + "%N")
			-- v.inc
      -- print ("even after wait%N")
      i := v.i
      -- print ("even after fetch%N")
      v.set_i (i + 1)
		end

  done: BOOLEAN

  is_done: BOOLEAN
    do
      Result := done
    end
end
