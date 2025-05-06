#include "philo.h"  // здесь определён t_state


void	ft_putstr(char *s)
{
	int	i;

	if (!s)
		return;
	i = 0;
	while (s[i])
	{
		write(1, &s[i], 1);
		i++;
	}
}

void	ft_putchar(char c)
{
	write(1, &c, 1);
}

void	ft_putnbr(int n)
{
	if (n == -2147483648)
	{
		ft_putstr("-2147483648");
		return;
	}
	if (n < 0)
	{
		ft_putchar('-');
		n = -n;
	}
	if (n >= 10)
		ft_putnbr(n / 10);
	ft_putchar((n % 10) + '0');
}

void	ft_pr(int time, int philo_id, char *str, t_state *state)
{
	pthread_mutex_lock(&state->finish_mutex);
	pthread_mutex_lock(&state->print_mutex);
	if (!state->finish)
	{
		if(philo_id == -1)
		{
			ft_putstr(str);
			write(1,"\n",1);
		}
		else
		{
			ft_putnbr(time - state->start_time);
			write(1," ",1);
			ft_putnbr(philo_id + 1);
			write(1," ",1);
			ft_putstr(str);
			write(1,"\n",1);
		}
	}
	pthread_mutex_unlock(&state->print_mutex);
	pthread_mutex_unlock(&state->finish_mutex);
}

long timestamp(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	ft_usleep(long ms)
{
	long	start;

	start = timestamp();
	while (timestamp() - start < ms)
		usleep(10);
}

int ft_atoi(const char *str)
{
	int i = 0;
	int sign = 1;
	int result = 0;

	// Skip whitespace characters
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;

	// Handle sign
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}

	// Convert digits to integer
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	
	return (result * sign);
}

t_p	*init(int i, int time_to_die_user, int sleep, int eat)
{
	t_p		*new;
	new = malloc(sizeof(t_p));
	if(!new)
		return(NULL);
	long time = timestamp();
	new->id = i;
	new->meals = 0;
	new->last_meal_time = time;
	if (pthread_mutex_init(&new->fork, NULL) != 0)
	{
		printf("Ошибка инициализации mutex у философа %d\n", i);
		free(new);
		return NULL;
	}
	if (pthread_mutex_init(&new->meal_mutex, NULL) != 0)
	{
		printf("Ошибка инициализации meal_mutex у философа %d\n", i);
		pthread_mutex_destroy(&new->fork);
		free(new);
		return NULL;
	}
	return(new);
}

void	free_p(t_p **p, int i)
{
	while (i >= 0)
	{
		free(p[i]);
		i--;
	}
	free(p);
}

void	ft_eat(long time, t_p *p)
{
	pthread_mutex_lock(&p->meal_mutex);
	p->last_meal_time = timestamp();
	p->meals++;
	pthread_mutex_unlock(&p->meal_mutex);
	ft_usleep(time);
}

void	init_forks(t_p **p, int size)
{
	if(!p)
		return;
	if(size == 1)
	{
		p[0]->left_p = NULL;
		return;
	}
	int i = 0;
	while (i < size - 1)
	{
		p[i]->left_p = p[i + 1];
		i++;
	}
	p[size - 1]->left_p = p[0];
}

static void	lock_forks(t_p *p)
{
	// if (p->id % 2 == 0)
	// 	usleep(100);
	pthread_mutex_lock(&p->fork);
	ft_pr(timestamp(), p->id, "has taken a fork", p->state);
	pthread_mutex_lock(&p->left_p->fork);
	ft_pr(timestamp(), p->id, "has taken a fork", p->state);
}

// void take_forks_eat(t_p *philosopher) {
// 	if(philosopher->left_p == NULL)
// 	{
// 		pthread_mutex_lock(&philosopher->fork);
// 		ft_pr(timestamp(), philosopher->id, "has taken a fork",philosopher->state);
// 		ft_usleep(philosopher->stats->time_to_die*2);
// 		return;
// 	}
// 	else if (philosopher->id % 2 == 0)
// 	{
// 		ft_usleep(10);
// 		pthread_mutex_lock(&philosopher->left_p->fork);
// 		ft_pr(timestamp(), philosopher->id, "has taken a fork neib fork",philosopher->state);
// 		pthread_mutex_lock(&philosopher->fork);
// 		ft_pr(timestamp(), philosopher->id, "has taken a fork",philosopher->state);
// 	}
// 	else
// 	{
// 		pthread_mutex_lock(&philosopher->fork);
// 		ft_pr(timestamp(), philosopher->id, "has taken a fork",philosopher->state);
// 		pthread_mutex_lock(&philosopher->left_p->fork);
// 		ft_pr(timestamp(), philosopher->id, "has taken a neib fork",philosopher->state);
// 	}

// 	ft_pr(timestamp(), philosopher->id, "is eating",philosopher->state);
// 	ft_eat(philosopher->stats->time_to_eat, philosopher);

// 	pthread_mutex_unlock(&philosopher->fork);
// 	pthread_mutex_unlock(&philosopher->left_p->fork);
// }


static void	unlock_forks(t_p *p)
{
	pthread_mutex_unlock(&p->fork);
	pthread_mutex_unlock(&p->left_p->fork);
}


void take_forks_eat(t_p *p) {
    if (!p->left_p) {
        // Edge case: Only 1 philosopher (can't eat)
        pthread_mutex_lock(&p->fork);
        ft_pr(timestamp(), p->id, "has taken a fork", p->state);
        ft_usleep(p->stats->time_to_die * 2); // Wait until death
        pthread_mutex_unlock(&p->fork);
        return;
    }

    lock_forks(p); // Ordered locking (deadlock-free)
    ft_pr(timestamp(), p->id, "is eating", p->state);
    ft_eat(p->stats->time_to_eat, p);
    unlock_forks(p); // Release in reverse order (optional)
}

void *life(void *arg) {
	t_p *philo = (t_p *)arg;
	t_state *state = philo->state;
	t_stats *stats = philo->stats;

	if(philo->id % 2 == 0)
		ft_usleep(100);
	while (1) {
		// Проверка на смерть перед началом цикла
		pthread_mutex_lock(&state->finish_mutex);
		if (state->finish) {
			pthread_mutex_unlock(&state->finish_mutex);
			break;
		}
		pthread_mutex_unlock(&state->finish_mutex);
		take_forks_eat(philo);
		ft_pr(timestamp(), philo->id, "is sleeping",philo->state);
		ft_usleep(stats->time_to_sleep);
		ft_pr(timestamp(), philo->id, "is thinking",philo->state);
	}
	return NULL;
}

int	meals_checker(t_p **philos)
{
	int i = 0;
	int size = philos[0]->stats->size;
	int m_counter = 0;
	t_state *state = philos[0]->state;
	while (i < size)
	{
		pthread_mutex_lock(&philos[i]->meal_mutex);
		if(philos[i]->meals >= philos[0]->stats->total_meals)
			m_counter++;
		pthread_mutex_unlock(&philos[i]->meal_mutex);
		i++;
	}
	if(size == m_counter)
	{
		ft_pr(timestamp(), -1,"ALL FINISHED", state);
		pthread_mutex_lock(&state->finish_mutex);
		if (!state->finish)
		{
			state->finish = 1;
		}
		pthread_mutex_unlock(&state->finish_mutex);
		return(1);
	}
	return (0);	
}

void *monitoring(void *arg) {
	t_p **philos = (t_p **)arg;
	t_state *state = philos[0]->state;
	while (1) {
		if(philos[0]->stats->total_meals != -1)
			if(meals_checker(philos))
				return NULL;
		for (int i = 0; i < philos[0]->stats->size; i++) {
			pthread_mutex_lock(&philos[i]->meal_mutex);
			long time_since_eat = timestamp() - philos[i]->last_meal_time;
			pthread_mutex_unlock(&philos[i]->meal_mutex);
			if (time_since_eat >= philos[i]->stats->time_to_die)
			{
				ft_pr(timestamp(), philos[i]->id,"died", philos[i]->state);
				pthread_mutex_lock(&state->finish_mutex);
				if (!state->finish)
					state->finish = 1;
				pthread_mutex_unlock(&state->finish_mutex);
				return NULL;
			}
		}
		usleep(1000);
	}
	return NULL;
}


t_stats *init_stats(int v, char **c)
{
	t_stats *stats = malloc(sizeof(t_stats));
	if(!stats)
		return (NULL);
	stats->size = ft_atoi(c[1]);
	stats->time_to_die = ft_atoi(c[2]);
	stats->time_to_eat = ft_atoi(c[3]);
	stats->time_to_sleep = ft_atoi(c[4]);
	if(v == 6)
		stats->total_meals = ft_atoi(c[5]);
	else
		stats->total_meals = -1;
	return(stats);
}
int	checker(char **argv, int i)
{
	i--;
	if(ft_atoi(argv[1]) == 0 || ft_atoi(argv[1]) < 1)
		return(0);
	while (i != 1)
	{
		if(ft_atoi(argv[i]) < 0)
			return(0);
		i--;
	}
	return(1);
}

int check_input(int argc, char **argv)
{
	if(argc != 5 && argc != 6)
		return (0);
	if(!checker(argv, argc))
		return(0);
	return (1);
}

t_state *init_state()
{
	t_state *state = malloc(sizeof(t_state));
	if(!state)
		return NULL;
	pthread_mutex_init(&state->finish_mutex, NULL);
	pthread_mutex_init(&state->print_mutex, NULL);
	state->finish = 0;
	state->start_time = timestamp();
	return state;
}

t_p	**init_philo(t_stats *stats, t_state *state)
{
	t_p **p = malloc(sizeof(t_p *) * stats->size);
	if(!p)
		return(NULL);
	for (int i = 0; i < stats->size; i++)
	{
		p[i] = init(i, stats->time_to_die,stats->time_to_sleep,stats->time_to_eat);
		p[i]->state = state;
		p[i]->stats = stats;
	}
	init_forks(p, stats->size);
	return(p);
}

void clear_all(t_stats *stats, t_state *state, t_p	**p)
{
	int i = 0;
	while (i < stats->size)
	{
		pthread_mutex_destroy(&p[i]->fork);
		pthread_mutex_destroy(&p[i]->meal_mutex);
		free(p[i]);
		i++;
	}

	free(p);
	free(stats);
	pthread_mutex_destroy(&state->finish_mutex);
	pthread_mutex_destroy(&state->print_mutex);
	free(state);
}
int	main(int argc, char **argv)
{
	if(!check_input(argc, argv))
		return 0;

	t_stats *stats = init_stats(argc, argv);
	t_state *state = init_state();

	t_p **p = init_philo(stats,state);

	pthread_t threads[stats->size];
	for (int i = 0; i < stats->size; i++)
		pthread_create(&threads[i], NULL, life, p[i]);

	pthread_t monitor_thread;
	pthread_create(&monitor_thread, NULL, monitoring, (void *)p);

	for (int i = 0; i < stats->size; i++)
		pthread_join(threads[i], NULL);
	pthread_join(monitor_thread, NULL);

	clear_all(stats,state,p);
	return 0;
}

// input check --> init all --> run all ---> clean all --> done