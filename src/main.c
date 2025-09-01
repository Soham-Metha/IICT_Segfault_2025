#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

const char *inputFile = (void *)0;
const char *outputFile = (void *)0;

void usage(const char *progName)
{
	print(WIN_STDOUT,
	      "Usage: %s [OPTIONS]                      \n"
	      "Options:                                 \n"
	      "    -i <input>        Provide input path \n"
	      "    -o <output>       Provide output path\n"
	      "    -h                Provide this helper\n"
	      "\n",
	      progName);
	exit(1);
}

char *getNextCmdLineArg(int *argc, char ***argv)
{
	assert(*argc > 0);

	char *arg = **argv;

	*argc -= 1;
	*argv += 1;

	return arg;
}

void processFlag(const char *prog, const char *flag, int *argc, char ***argv)
{
	switch (flag[1]) {
	case 'i':
		inputFile = getNextCmdLineArg(argc, argv);
		return;
	case 'o':
		outputFile = getNextCmdLineArg(argc, argv);
		return;
	case 'h':
	default:
		usage(prog);
	}
}

int main(int argc, char **argv)
{
	const char *program = getNextCmdLineArg(&argc, &argv);

	while (argc > 0) {
		const char *flag = getNextCmdLineArg(&argc, &argv);
		processFlag(program, flag, &argc, &argv);
	}

	if (!inputFile || !outputFile) {
		usage(program);
	}

	return 0;
}
