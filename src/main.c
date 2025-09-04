#include <Wrapper/IO.h>
#include <Layer_File.h>
#include <Wrapper/UI.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

const char *inputFile = (void *)0;
const char *outputFile = (void *)0;
int use_tui = 0;

static void usage(const char *progName)
{
	print(WIN_STDOUT,
	      "Usage: %s [OPTIONS]                      \n"
	      "Options:                                 \n"
	      "    -i <input>        Provide input path \n"
	      "    -o <output>       Provide output path\n"
	      "    -tui              Launch ncurses TUI \n"
	      "    -h                Provide this helper\n"
	      "\n",
	      progName);
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
		usage(prog);
		exit(0);
	case 't':
		use_tui = 1;
		return;
	default:
		usage(prog);
		exit(1);
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
		exit(1);
	}

	File_Context ctx = { 0 };
	file_read(inputFile, &ctx);

	if (use_tui) {
		// TUI *tui = init_ui(&ctx);
		// run_ui(tui);
		// destroy_ui(tui);
		onStartup(&ctx);
		onShutdown();
		return 0;
	}

	print(WIN_STDOUT, "\n\n");
	return 0;
}
