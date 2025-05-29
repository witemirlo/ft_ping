NAME     := ft_ping

SRCS     := src/main.c\
	    src/connection_data.c\
	    src/icmp.c\
	    src/parser.c\
	    src/print_header.c\
	    src/routines.c\
	    src/signal.c\

OBJS     := $(SRCS:.c=.o)

CC       := cc
CFLAGS   := -Wall -Wextra -O0 -g3 -pedantic -Wformat=2 -Wformat-overflow=2
SANITIZE := -fsanitize=address,pointer-compare,pointer-subtract,leak,undefined,bounds-strict,float-divide-by-zero,float-cast-overflow

CPPFLAGS := -I include/

all: $(NAME)
re: fclean all

$(NAME): $(OBJS) include/ft_ping.h
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	make set_capabilities

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

sanitize:
	export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1
	make re CC=gcc CFLAGS="$(CFLAGS) $(SANITIZE)"

set_capabilities:
	sudo setcap 'cap_net_raw=ep' $(shell pwd)/${NAME}

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

.SECONDARY: $(OBJS)
.PHONY: all clean fclean re set_capabilities
