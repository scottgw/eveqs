note
	description: "Summary description for {VAR}."
	author: ""
	date: "$Date$"
	revision: "$Revision$"

class
	VAR

feature
	i: INTEGER

	set_i (a_i: INTEGER)
		do
			i := a_i
      -- print ("Var: " + i.out + "%N")
		end

end
