/*
 * Implementation of optparse.h
 *
 * W. Evan Sheehan <evan_sheehan@nrel.gov>
 *
 */

#include "optparse.h"

#include <iostream>
#include <unistd.h>

using namespace std;

/********************** Option **********************/

Option::Option (string shrt, string lng, string dest,
		string hlp, action_t act):
	action (act),
	shrt_flag (shrt),
	lng_flag (lng),
	help (hlp),
	destination (dest)
{
}

Option::~Option ()
{
}

bool
Option::operator == (const string & option)
{
	return (shrt_flag == option || lng_flag == option);
}

/******************** OptionParser *******************/

OptionParser::OptionParser (string usage, bool show_opts):
	help_msg_show_opts (show_opts),
	use_msg (usage)
{
  // Set the length of the longest flag
  lng_flag_max_len = strlen("--help");
  shrt_flag_max_len = strlen("-?");

	/* help option is default, it has no destination. */
	opts.insert(opts.begin(), Option("-?","--help","", "display this message"));
}

OptionParser::~OptionParser ()
{
}

void
OptionParser::add_option (string shrt_flag, string lng_flag, string destination,
    string help, string dfault,  action_t act)
{
	/* Add the option to our list of options. */
	opts.insert(opts.begin(),Option(shrt_flag, lng_flag, destination, help, act));

	/* Set the default value for this option, this not only allows you to
	 * set default values but insures that every option's destination will
	 * be in our dictionary, even if the value is only "".
	 */
	options[destination] = dfault;

  /* If either flag for this option is the largest flag record its length for
   * formatting the help message.
   */
  if (strlen(lng_flag.c_str()) > lng_flag_max_len)
    lng_flag_max_len = strlen(lng_flag.c_str());
  if (strlen(shrt_flag.c_str()) > shrt_flag_max_len)
    shrt_flag_max_len = strlen(shrt_flag.c_str());
}

void
OptionParser::parse_args (int argc, char **argv)
{
	/* Walk through the arguments and sort them into options
	 * or arguments.
	 */
	for (int i = 1; i < argc; i++) {
		/* If we start with a '-' we're probably a <flag><value> pair
		 * so we need to figure out which option it is. find_opt() is
		 * where the real work is done.
		 */
		if (argv[i][0] == '-')
			find_opt(argv, i);

		/* If we're looking at an argument (i.e. a value with no flag who's
		 * meaning is determined by position) just append it into the
		 * arguments list.
		 */
		else
			arguments.insert(arguments.end(), argv[i]);
	}
}

void
OptionParser::error (string msg)
{
	cerr << use_msg << endl << msg << endl;
	exit(1);
}

void
OptionParser::help ()
{
	cout << use_msg << endl;
	if (help_msg_show_opts) {
		for (unsigned int i=0; i < opts.size(); i++)
			printf("  %-*s %-*s  %s\n", shrt_flag_max_len+1, opts[i].shrt_flag.c_str(),
          lng_flag_max_len+2, opts[i].lng_flag.c_str(),
					opts[i].help.c_str());
	}
	exit(0);
}

void
OptionParser::find_opt(char **argv, int &index)
{
	/* If it's the built in help option we'll just print the help message
	 * and quit.
	 */
	if ((string)argv[index] == "-?" || (string)argv[index] == "--help")
		help();

	/* Step through our list of known options. */
	for (unsigned int i = 0; i < opts.size(); i++) {
		/* Uses the overridden == operator for the Options class
		 * to compare argv[index] to the flags of each Option.
		 */
		if (opts[i] == (string)argv[index]) {
			switch (opts[i].action) {
				case STORE_FALSE:
					options[opts[i].destination] = "0";
					break;
				case STORE_TRUE:
					options[opts[i].destination] = "1";
					break;
				case STORE:
					/* Set the value and return if we've found a match. */
					options[opts[i].destination] = argv[index+1];
					index++;
					break;
				default:
					break;
			};
			return;
		}
	}

	/* If we haven't found a match this is not a known argument. */
	error("Error: invalid argument");
}
/*** The End ***/
