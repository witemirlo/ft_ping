NAME     := ft_ping

SRCS     := src/main.c\
	    src/connection_data.c\
	    src/icmp.c\
	    src/init.c\
	    src/parser.c\
	    src/payload.c\
	    src/print_header.c\
	    src/routines.c\
	    src/signal.c\

OBJS     := $(SRCS:.c=.o)

CC       := cc
CFLAGS   := -Wall -Wextra -O0 -g3 -pedantic -Wformat=2 -Wformat-overflow=2 -fsanitize=address,pointer-compare,pointer-subtract,leak,undefined,bounds-strict,float-divide-by-zero,float-cast-overflow
CPPFLAGS := -I include/

VFLAGS   := -Wall -Wextra -O0 -g3 -pedantic -Wformat=2 -Wformat-overflow=2
TFLAGS   := google.com

TTL      := 64

# ------------------------------------------------------------------------------

all: $(NAME)
re: fclean all

$(NAME): $(OBJS) include/ft_ping.h
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
ifneq ($(CFLAGS), $(VFLAGS))
	make set_capabilities
endif

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

valgrind:
	make re CFLAGS="$(VFLAGS)"
	sudo valgrind --track-fds=yes --leak-check=full --show-leak-kinds=all --trace-children=yes ./$(NAME) $(TFLAGS)

set_capabilities:
	sudo setcap 'cap_net_raw=ep' $(shell pwd)/${NAME}

set_ttl:
	sudo sysctl -w net.ipv4.ip_default_ttl=$(TTL)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

.SECONDARY: $(OBJS)
.PHONY: all clean fclean re set_capabilities valgrind
