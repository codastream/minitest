NAME		:=	tester
LIBFT		:=	libft.a

#==============================COMPIL===========================#

CC:=		cc
CFLAGS:=	-Wall -Wextra -Werror -g

ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

#================================DIRS============================#

#==============================SOURCES===========================#

SRCS_FILES:=	test.c\

SRCS:=			$(SRCS_FILES)

#=============================OBJECTS===========================#

OBJS:=			$(SRCS:%.c=%.o)

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	@$(CC) $(CFLAGS) -o $@  $(OBJS) -L. -lft

%.o: %.c
	@$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	rm -f $(LIBFT)

re: fclean all

.PHONY: all clean fclean re
