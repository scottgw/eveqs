note
	description: "Summary description for {VAR}."
	author: ""
	date: "$Date$"
	revision: "$Revision$"

class
	VAR

feature
	i: INTEGER

	update_var
		do
			i := i + 1 -- fib (30)
		end
end
