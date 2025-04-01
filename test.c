#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../libft/includes/libft.h"

# define MINISHELL_PATH "../minishell"
# define OUTDIR_MINI "outdir_m"
# define OUTDIR_BASH "outdir_b"
# define OUTFIR "./files"
# define OUTFILE "outfile.txt"
# define OUTFILE_MINI "./outdir_m/outfile.txt"
# define OUTFILE_BASH "./outdir_b/outfile.txt"

// # define MINISHELL_PATH "./minishell"

char	*escape_quotes(char *s)
{
	char	*s1;
	char	*s2;
	char	*s3;

	s1 = ft_subst(s, "\"", "\\");
	s2 = ft_subst(s1, "'", "\\'");
	s3 = ft_subst(s2, "$", "\\$");
	return (s3);
}

char	*build_test_part_for_single(char *test)
{
	char **ins_part;
	char *test_part;

	ins_part = ft_calloc(4, sizeof(char *));
	ins_part[0] = "echo \"\"\"";
	ins_part[1] = test;
	ins_part[2] = " \"\"\" | ";
	ins_part[3] = NULL;
	test_part = ft_multistrjoin(3, ins_part, "");
	free(ins_part);
	return (test_part);
}

char	*build_test_part(char *test)
{
	char **ins_part;
	char *test_part;

	ins_part = ft_calloc(4, sizeof(char *));
	ins_part[0] = "echo '";
	ins_part[1] = test;
	ins_part[2] = "' | ";
	ins_part[3] = NULL;
	test_part = ft_multistrjoin(3, ins_part, "");
	free(ins_part);
	return (test_part);
}

char	*build_instruction(char shell_type, char *test, char *run_mode)
{
	char **ins_parts;
	char *instruction;

	ins_parts = ft_calloc(4, sizeof(char *));
	if (ft_strchr(test, '"'))
		ins_parts[0] = build_test_part(test);
	else
		ins_parts[0] = build_test_part_for_single(test);
	if (shell_type == 'm')
		ins_parts[1] = MINISHELL_PATH;
	else
		ins_parts[1] = "bash";
	if (!ft_strcmp(run_mode, "system"))
		ins_parts[2] = " 2> /dev/null 1> /dev/null";
	else
		ins_parts[2] = " 2>1";
	ins_parts[3] = NULL;
	instruction = ft_multistrjoin(3, ins_parts, "");
	free(ins_parts);
	return (instruction);
}

void	print_stream_to_file(FILE *out, int fd)
{
	char	buffer[1000] = {0};
	while (fgets(buffer, sizeof(buffer)-1, out))
	{
		ft_printfd(fd, (const char *) buffer);
	}
}

void	print_stream_to_string(char shell, int ret, FILE *out, char *buffer)
{
	size_t n;

	(void) shell;
	(void) ret;
	n = fread(buffer, 1, 999, out);
	buffer[n] = '\0';
}
int	get_return_status(int ret_system)
{
	int ret;

	if (WIFEXITED(ret_system))
		ret = WEXITSTATUS(ret_system);
	else
		ret = -1;
	return (ret);
}

bool	have_same_return(int ret_b, int ret_m)
{
	if (ret_b != ret_m)
	{
		return (false);
	}
	return (true);
}

bool	have_same_output(int ret_b, int ret_m, char *buff_b, char *buff_m)
{
	char *bash_with_prompt;

	(void) ret_m;
	// only get first line for bash output
	char *newline = ft_strchr(buff_b, '\n');
	if (newline != NULL)
	{
		*newline = '\0';
	}
	newline = ft_strchr(buff_m, '\n');
	if (newline != NULL)
	{
		*newline = '\0';
	}
	if (ft_strstr(buff_b, "not found"))
	{
		if (ft_strstr(buff_m, "not found"))
			return (true);
		else
			return (false);
	}
	else if (ft_strstr(buff_b, "syntax error near unexpected token"))
	{
		if (ft_strstr(buff_m, "syntax error near unexpected token"))
			return (true);
		else
			return (false);
	}
	else if (ft_strstr(buff_b, "No such file or directory"))
	{
		if (ft_strstr(buff_m, "No such file or directory"))
			return (true);
		else
			return (false);
	}
	if (ret_b != 0)
	{
		bash_with_prompt = ft_strstr(buff_b, ": line ");
		if (bash_with_prompt)
		{
			bash_with_prompt += ft_strlen(": line ");
			while (*bash_with_prompt != ':')
				bash_with_prompt++;
			bash_with_prompt += 2;
			buff_b = bash_with_prompt;
		}
		if (ft_strstr(buff_m, buff_b))
			return (true);
		else
			return (false);
	}

	if (ft_strcmp(buff_b, buff_m))
	{
		return (false);
	}
	return (true);
}
void	init_redir(void)
{
	chmod("./files/badperm.txt", 0);
	FILE *filestream = fopen("./files/outfile.txt", "w");
	if (!filestream)
		printf("error creating outfile.txt\n");
	fclose(filestream);
	mkdir(OUTDIR_MINI, 0755);
	mkdir(OUTDIR_BASH, 0755);
}

void	reset_redir(void)
{
	int	code;

	if (access("./files/inexistent", F_OK) == 0)
	{
		code = remove("./files/inexistent");
		if (code != 0)
			printf("code remove inexistent %d\n", code);
	}
	code = remove("./files/outfile.txt");
	if (code != 0)
		printf("code remove outfile %d\n", code);
	chmod("./files/badperm.txt", 0755);
	rmdir(OUTDIR_MINI);
	rmdir(OUTDIR_BASH);
	if (access(OUTFILE_MINI, F_OK))
		remove(OUTFILE_MINI);
	if (access(OUTFILE_BASH, F_OK))
		remove(OUTFILE_BASH);
}

void	print_file(char *file)
{
	int	fd;
	char	*line;

	fd = open(file, O_RDONLY);
	line = get_next_line(fd);
	while (line)
	{
		printf("%s", line);
	}
	free(line);
	close(fd);
}

bool	is_identical_outfile(void)
{
	int	fd_m;
	int	fd_b;
	char	*line_m;
	char	*line_b;

	fd_m = open(OUTFILE_MINI, O_RDONLY);
	fd_b = open(OUTFILE_BASH, O_RDONLY);
	line_m = get_next_line(fd_m);
	line_b = get_next_line(fd_b);
	while (line_m)
	{
		if (ft_strcmp(line_m, line_b))
		{
			free(line_m);
			free(line_b);
			close(fd_m);
			close(fd_b);
			return (false);
		}
		free(line_m);
		free(line_b);
		line_m = get_next_line(fd_m);
		line_b = get_next_line(fd_b);
	}
	free(line_m);
	free(line_b);
	close(fd_m);
	close(fd_b);
	return (true);
}

void	do_tests_for_file(int fd, int *test_index, int *ok_count, bool print_output, bool print_only_failed, char *filename)
{
	char	*test;
	int		ret_system;
	int		ret_m;
	int		ret_b;
	char	*buff_m = ft_calloc(1000, sizeof(char));
	char	*buff_b = ft_calloc(1000, sizeof(char));
	FILE	*outstream_m;
	FILE	*outstream_b;
	char	*instruction_m;
	char	*instruction_popen_m;
	char	*instruction_b;
	char	*instruction_popen_b;
	bool	is_same_return = false;
	bool	is_same_output = false;
	bool	is_same_outfile = false;
	bool	is_passed = false;

	test = get_next_line(fd);
	if (test && ft_strlen(test) > 0)
		test[ft_strlen(test) - 1] = '\0';
	printf("%s------------------------------------------------------------%s\n", P_TEAL, P_NOC);
	printf("%s\n", filename);
	printf("%s------------------------------------------------------------%s\n", P_TEAL, P_NOC);
	while (test)
	{
		// ===== INIT
		init_redir();

		// ===== COMPARE RETURN
		instruction_b = build_instruction('b', test, "system");
		ret_system = system(instruction_b);
		ret_b = get_return_status(ret_system);
		if (access("./files/outfile.txt", F_OK))
		{
			rename("./files/outfile.txt", OUTFILE_BASH);
		}
		instruction_m = build_instruction('m', test, "system");
		ret_system = system(instruction_m);
		ret_m = get_return_status(ret_system);
		if (access("./files/outfile.txt", F_OK))
		{
			rename("./files/outfile.txt", OUTFILE_MINI);
		}
		// printf("ret code %d\n", ret_m);

		// ===== COMPARE OUTPUT
		instruction_popen_m = build_instruction('m', test, "popen");
		instruction_popen_b = build_instruction('b', test, "popen");
		outstream_m = popen(instruction_popen_m, "r");
		print_stream_to_string('m', ret_m, outstream_m, buff_m);
		outstream_b = popen(instruction_popen_b, "r");
		print_stream_to_string('n', ret_b, outstream_b, buff_b);
		pclose(outstream_m);
		pclose(outstream_b);

		// ===== COMPARE FILES
		if (access(OUTFILE_BASH, F_OK))
			is_same_outfile = is_identical_outfile();
		else
			is_same_outfile = true;

		// ===== DISPLAY

		is_same_return = have_same_return(ret_b, ret_m);
		is_same_output = have_same_output(ret_b, ret_m, buff_b, buff_m);
		is_passed = is_same_return && is_same_output && is_same_outfile;
		if (!print_only_failed || (print_only_failed && !is_passed))
			printf("%d : ", *test_index);
		if (is_passed)
		{
			if (!print_only_failed)
				printf("✅");
			(*ok_count)++;
		}
		else
		{
			printf("❌");
		}
		if (!print_only_failed || (print_only_failed && !is_passed))
			printf("\t%s%50s%s\n", P_YELLOW, test, P_NOC);
		if (!is_same_output || print_output)
		{
			printf("❕\tsortie\nbash:%s\nmini:%s\n", buff_b, buff_m);
		}
		if (!is_same_return)
		{
			printf("⏹️\tvaleurs de retour\nbash: %d\nmini: %d\n", ret_b, ret_m);
		}
		if (!is_same_outfile)
		{
			printf("📁\toutfile.txt\n");
			printf("bash:\n");
			print_file(OUTFILE_BASH);
			printf("mini:\n");
			print_file(OUTFILE_MINI);
		}
		if (!print_only_failed || (print_only_failed && !is_passed))
			printf("%s------------------------------------------------------------%s\n", P_BLACK, P_NOC);

		// ===== CLEANUP
		reset_redir();
		free(test);

		(*test_index)++;
		test = get_next_line(fd);
	}
	free(test);
}

void	fill_test_files(char **test_files, char **av)
{
	if (!ft_strcmp("syntax", av[1]))
		test_files[0] = "tests/00_syntax.txt";
	else if (!ft_strcmp("builtins", av[1]))
		test_files[0] = "tests/01_builtins.txt";
	else if (!ft_strcmp("vars", av[1]))
		test_files[0] = "tests/02_vars.txt";
	else if (!ft_strcmp("commands", av[1]))
		test_files[0] = "tests/03_commands.txt";
	else if (!ft_strcmp("redirs", av[1]))
		test_files[0] = "tests/04_redirs.txt";
	else if (!ft_strcmp("pipes", av[1]))
		test_files[0] = "tests/05_pipes.txt";
	else if (!ft_strcmp("wildcard", av[1]))
		test_files[0] = "tests/07_bonus_wildcard.txt";
}

void	fill_all_test_files(char **test_files)
{
	test_files[0] = "tests/00_syntax.txt";
	test_files[1] = "tests/01_builtins.txt";
	test_files[2] = "tests/02_vars.txt";
	test_files[3] = "tests/03_commands.txt";
	test_files[4] = "tests/04_redirs.txt";
	test_files[5] = "tests/05_pipes.txt";
	test_files[6] = "tests/02_dollars.txt";
	test_files[7] = "tests/02_quotes.txt";
	test_files[8] = "tests/07_bonus_wildcard.txt";
}

int main(int ac, char **av)
{
	char	**test_files;
	int		i;
	int		fd;
	int		ok_count;
	int		test_count;
	bool	print_output;
	bool	print_only_failed;

	print_output = false;
	print_only_failed = false;
	ok_count = 0;
	test_count = 0;
	test_files = ft_calloc(10, sizeof(char *));

	if (ac == 1)
	{
		fill_all_test_files(test_files);
	}
	else if (ac == 2)
	{
		fill_test_files(test_files, av);
	}
	else if (ac == 3)
	{
		if (ft_strcmp(av[1], "all"))
			fill_test_files(test_files, av);
		else
		{
			fill_all_test_files(test_files);
		}
		if (av[2][0] == 'p')
			print_output = true;
		if (av[2][0] == 'f')
		{
			print_only_failed = true;
		}
	}
	else
	{
		printf("usage : \n./tester\n./tester <filename (one of syntax - builtins - vars - commands - redirs - pipes)>\n./tester <filename> <print (one of 'p' : print all tests and all result (whether test is passed or failed) - 'f' : print only failed tests)\n");
		return (1);
	}

	i = 0;
	while (test_files[i])
	{
		fd = open(test_files[i], O_RDONLY, 0666);
		if (fd < 0)
			ft_printfd(2, "%sErreur d'ouverture du fichier %s%s\n", P_RED, test_files[i], P_NOC);
		do_tests_for_file(fd, &test_count, &ok_count, print_output, print_only_failed, test_files[i]);
		close(fd);
		i++;
	}
	if (ok_count == test_count)
	{
		printf("🎊 Success ");
		printf("%d out of %d tests passed", ok_count, test_count);
		return (EXIT_SUCCESS);
	}
	else
	{
		printf("👷 Work still needed ");
		printf("%d out of %d tests passed", ok_count, test_count);
		return (EXIT_FAILURE);
	}
}

