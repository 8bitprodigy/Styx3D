/*
   eos.script library
   Copyright (C) 2004 Fabian Dannenberger

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



   The original version is located at:
   http://www.eosscript.org

   Fabian Dannenberger
   info@eosscript.org
*/

#ifndef EOSFUNCTION_H
#define EOSFUNCTION_H

/* v and s are argument stypes which can be stored by instructions. in the constructor
   they define what type of value they store. v ist just a text storage and s contains
   a variable (needed for static values of the script)  */
struct v
{
	v() : name("unkn"), type(0) {}
	v(string n)
	{ 
		name=n; 
		(n[0] == '$') ? type=1 : (n[0] == '%') ? type=2 : type=0;
		if(n[0] == '"') name.erase(name.begin());
	}
	string name;	// var name
	int		 type;	// var type: 1=global 2=local 0=static
};

/* static value param */
struct s
{
	s() : value(0) {}
	s(ptr<eosvariable> val) : value(val) {}
	ptr<eosvariable> value;
};

// pre defined for instr base
class eosfuncbase;
class eosfunction;
class eosobject;

/* this is the base class for instructions. we decl it here because eosfunction
	 needs to work with them. the real instructions are decl in eosinstr.h */
class eosinstr : public eosbase
{
public:
  // execute this instruction.. pure virtual
	virtual void exec(ptr<eosfunction> owner) = 0;
	string name;

	v one;	// store arguments
	s	temp;	// ..

	int l;
	string f;
};

/* this is a very basic version of the "real" vm. it contains the
	 stacks needed for execution of scripts (argstack, index stack)
	 and stores the current enviroment */
class eosexec : public eossingleton<eosexec>
{
public:
	eosexec(ptr<eosobject> ge);

	// reset index stack
	void	reset_index();
	/* push value to the stack. there are some premade eosvariable which are used for better
	   performance (no alloc in execution) */
	void	push(int i);
	void	push(float f);
	void	push(string s);
	void	push(ptr<eosfuncbase> o);
	void	push(ptr<eosobject> o);
	
	/* pop value from stack, returns variable */
	ptr<eosvariable> pop();

	/* request a temporty eosvariable */
	ptr<eosvariable> req_temp();
	
	/* returns stack size */
	int	stack_size();	

	/* pre-created variables for stack */
	vector< ptr<eosvariable> > temp;
	int	temppos;

	/* argument stack! main stack.. */
	stack< ptr<eosvariable> >	argstack;
	/* current enviroment */
	ptr<eosobject>						argenv;
	/* last enviroment */
	ptr<eosobject>						argrec;
	/* ptr to global enviroment */
	ptr<eosobject>						genvi;

	/* index stack (for var indices) */
	stack< ptr<eosvariable> >	argidx;
	ptr<eosvariable>					temp_idx;
};

#define exe eosexec::inst()

/* base function class. functions can be script fuunctions or
   cpp functions. this is just an interface, with some pure 
   virtual functions. already stores locals and bytecode */
class eosfuncbase : public eosbase
{
public:
	// these functions have to be used by all func types
	virtual void	exec(ptr<eosobject>	owner)		= 0;
	virtual void	mix(ptr<eosfuncbase> f)	= 0;
	virtual void	output_data()									= 0;

	string													name;		// name of function
	map<string, ptr<eosvariable> >	locals;	// map of locals
	vector< ptr<eosinstr> >					instr;	// bytecode
};

/* C-function structure. represents a c-function ported to the script 
   enviroment. you can set a cpp method or a simple c function to this
   class. */
class eoscfunction : public eosfuncbase
{
public:
  // constructor for each type
	eoscfunction(string name) {type=0;}
	eoscfunction(string name, ptr<cmethod> method) {type=1; meth=method;}
	eoscfunction(string name, void (*f)()) {type=2; func=f;}

  // execute the function
	void	exec(ptr<eosobject>	owner) 
	{
		if(type==1)
			(*meth)();	// method
		if(type==2)
			(*func)();	// function
	}
	
	// c-function cant be mixed
	void	mix(ptr<eosfuncbase> f) {};
	// and got no output
	void	output_data() {};


private:
	void					(*func)();	// store c-function
	ptr<cmethod>	meth;				// or cpp method

	int						type;				// 1=meth 2=func

};

/* this class represents a script function in memory. its close to an object, you
   can access all locals like globals of an object. this class also stores the analysed
   bytecode of the function, plus methods to execute them. there are also some execution
   specific variables for recursive execution ... */
class eosfunction : public eosfuncbase
{
public:
	eosfunction(string s)
	{ name=s; log->add("function:0x%X('%s') created", this, s.c_str()); };
	eosfunction()
	{ name="unnamed"; log->add("function:0x%X('%s') created", this, name.c_str());};

	~eosfunction()
	{ log->add("function '%s' deleted", name.c_str()); };

	/* execute the script, pass owner (object from which executed) */
	void	exec(ptr<eosobject>	owner);
	/* prepare execution, called in exec (restores stacks, etc.) */
	void	prepare_exec();
	/* reset execution, called in exec (restores env, etc.) */
	void	reset_exec();
	/* sub tasks of reset_exec.. sometimes executed alone */
	void	reset_argenv();	// reset enviroment
	void	reset_argidx();	// reset indices
	void	reset_args();		// reset arguments

	/* locals are handles like globals of eosobject... refer to eosobject.h */
	void	copy_local(string local, ptr<eosvariable> o);
	void	set_local(string local, int idx, int value);
	void	set_local(string local, int idx, float value);
	void	set_local(string local, int idx, string value);
	void	set_local(string local, int idx, ptr<eosfunction> value);
	void	set_local(string local, int idx, ptr<eosobject> value);

	/* locals are handles like globals of eosobject... refer to eosobject.h */
	int			get_locali(string local, int idx);
	float		get_locald(string local, int idx);
	string	get_locals(string local, int idx);
	ptr<eosfunction>	get_localf(string local, int idx);
	ptr<eosobject>		get_localo(string local, int idx);
	ptr<eosvariable>	get_local(string local);

	/* take data from another function and mix them */
	void	mix(ptr<eosfuncbase> f);
	/* print bytecode to log */
	void	output_data();


public:
	// for execution
	int											curr_lines;				// current line of execution
	stack<int>							recursive_lines;	// stack, if this func is executed twice or more (recursive)
	ptr<eosobject>					last_owner;				// last owner, needed to reset enviroment	

	vector< ptr<eosvariable> >	params;				// list of pointer to parameter locals

	// flag for compare (bytecode)
	int										equal_flag;		
	int										size_flag;
};


#endif