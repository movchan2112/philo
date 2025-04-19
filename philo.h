#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <sys/time.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

/* ---------- Structures ---------- */
typedef struct s_p
{
	int					id;
	long				time;
	int					meals;
	long				last_meal_time;
	pthread_mutex_t 	fork;
	pthread_mutex_t 	meal_mutex;
	struct s_p 			*left_p;
	struct s_state		*state;
	struct s_stats		*stats;
} t_p;

typedef struct s_state {
	int finish;
    long start_time;
	pthread_mutex_t finish_mutex;
	pthread_mutex_t print_mutex;
} t_state;



typedef struct	s_stats
{
	int		time_to_die;
	int		time_to_eat;
	int		time_to_sleep;
	int		size;
	int		total_meals;
} t_stats;

void	ft_putstr(char *s);
void	ft_putnbr(int n);


#endif