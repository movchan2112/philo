# Executable name
NAME = philo

# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Source files
SRC = main.c

# Object files
OBJ = $(SRC:.c=.o)

# Header files
HEADERS = philosophers.h

# Default rule (Build project)
all: $(NAME)

# Rule to build the executable (Prevents unnecessary relinking)
$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

# Ensure object files are rebuilt if header files change
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Cleaning object files
clean:
	rm -f $(OBJ)

# Full clean (removes executable)
fclean: clean
	rm -f $(NAME)

# Rebuild project
re: fclean all

.PHONY: all clean fclean re