#include <uwsgi.h>

/*
	Spare2 cheaper algorithm, backported from uWSGI 2.1

	This algorithm is very similar to spare, but more suited for higher workload.

	This algorithm increase workers *before* overloaded, and decrease workers slowly.

	This algorithm uses these options: chaper, cheaper-initial, cheaper-step and cheaper-idle.

	* When number of idle workers is smaller than cheaper count, increase
	  min(cheaper-step, cheaper - idle workers) workers.
	* When number of idle workers is larger than cheaper count, increase idle_count.
		* When idle_count >= cheaper-idle, decrease worker.
*/


extern struct uwsgi_server uwsgi;


// this global struct containes all of the relevant values
static int cheaper_idle;

static struct uwsgi_option spare2_options[] = {
	{"cheaper-idle", required_argument, 0, "decrease workers after specified idle (algo: spare2) (default: 10)",
		uwsgi_opt_set_int, &cheaper_idle, UWSGI_OPT_MASTER | UWSGI_OPT_CHEAPER},
	{0, 0, 0, 0, 0, 0 ,0},
};


static int cheaper_algo_spare2(int can_spawn) {
	static int idle_count = 0;
	int i, idle_workers, busy_workers, cheaped_workers;

	// count the number of idle and busy workers
	idle_workers = busy_workers = 0;
	for (i = 1; i <= uwsgi.numproc; i++) {
		if (uwsgi.workers[i].cheaped == 0 && uwsgi.workers[i].pid > 0) {
			if (uwsgi_worker_is_busy(i) == 1) {
				busy_workers++;
			} else {
				idle_workers++;
			}
		}
	}
	cheaped_workers = uwsgi.numproc - (idle_workers + busy_workers);

#ifdef UWSGI_DEBUG
	uwsgi_log("cheaper-spare2: idle=%d, busy=%d, cheaped=%d, idle_count=%d\n",
		idle_workers, busy_workers, cheaped_workers, idle_count);
#endif

	// should we increase workers?
	if (idle_workers < uwsgi.cheaper_count) {
		int spawn;
		idle_count = 0;

		if (!can_spawn)
			return 0;

		spawn = uwsgi.cheaper_count - idle_workers;
		if (spawn > cheaped_workers)
			spawn = cheaped_workers;
		if (spawn > uwsgi.cheaper_step)
			spawn = uwsgi.cheaper_step;
		return spawn;
	}

	if (idle_workers == uwsgi.cheaper_count) {
		idle_count = 0;
		return 0;
	}

	// decrease workers
	idle_count++;
	if (idle_count < cheaper_idle)
		return 0;

	idle_count = 0;
	return -1;
}


// registration hook
static void register_algo(void) {
	// set defaults
	cheaper_idle = 10;

	uwsgi_register_cheaper_algo("spare2", cheaper_algo_spare2);
}

struct uwsgi_plugin cheaper_spare2_plugin = {
	.name = "cheaper_spare2",
	.on_load = register_algo,
	.options = spare2_options,
};

/* vim: set ts=8 sts=0 sw=0 noexpandtab : */
