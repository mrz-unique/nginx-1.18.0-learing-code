
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    void                     *sync;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};


struct ngx_cycle_s {
    void                  ****conf_ctx;						//配置上下文数组(含所有模块)
    /*  void ****conf_ctx 结构简化理解：
        ==> 
        typedef void**          void_array;         // 即void*[],存储void*的数组
        typedef void_array**    void_muti_array;    // 即void_array*[]
        void_multi_array        conf_ctx;           // 定义conf_ctx
        conf_ctx实际上是一个二维数组，对应模块的配置或者结构体存储。
        */

    ngx_pool_t               *pool;							//内存池

    ngx_log_t                *log;							//日志
    ngx_log_t                 new_log;						//中转的日志，在ngx_init_cycle的时候产生一个日志初始化，再 log = &new_log

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_uint_t                files_n;
    ngx_connection_t        **files;						//连接文件
    ngx_connection_t         *free_connections;				//空闲连接，起始地址
    ngx_uint_t                free_connection_n;			//空闲连接数量

    ngx_module_t            **modules;
    ngx_uint_t                modules_n;
    ngx_uint_t                modules_used;    /* unsigned  modules_used:1; */

    ngx_queue_t               reusable_connections_queue;
    ngx_uint_t                reusable_connections_n;

	/* 动态数组，每个数组元素存储着ngx_listening_t成员，表示监听端口和相关的参数 */
    ngx_array_t               listening;
	
	/* 动态数组容器，保存Nginx所有要操作的目录。如果有目录不存在，则会试图创建，而创建目录失败将会导致
	Nginx启动失败。例如，上传文件的临时目录也在pathes中，如果没有权限创建，则会导致Nginx无法启动 */
    ngx_array_t               paths;

    ngx_array_t               config_dump;
    ngx_rbtree_t              config_dump_rbtree;
    ngx_rbtree_node_t         config_dump_sentinel;

	/* 单链表容器，元素类型时ngx_open_file_t结构体，表示Nginx已经打开的所有文件。事实上，Nginx框架不会
	向open_files链表中添加文件，而是由对此感兴趣的模块向其中添加文件路径名，Nginx框架会在ngx_init_cycle
	方法中打开这些文件 */
    ngx_list_t                open_files;

	/* 单链表容器，元素类型是ngx_shm_zone_t结构体，每个元素表示一块共享内存 */
    ngx_list_t                shared_memory;

	/* 当前进程所有连接对象的总数，与下面的connections成员配合使用 */
    ngx_uint_t                connection_n;
	/* 指向当前进程中的所有连接对象 */
    ngx_connection_t         *connections;

	/* 指向当前进程中的所有读事件对象，connection_n同时表示所有读事件的总数 */
    ngx_event_t              *read_events;
	/* 指向当前进程中的所有写事件对象，connection_n同时表示所有写事件的总数 */
    ngx_event_t              *write_events;

	/* 旧的ngx_cycle_t对象用于引用上一个ngx_cycle_t对象中的成员。例如ngx_init_cycle方法，在启动初期，需
	要建立一个临时的ngx_cycle_t对象保存一些变量，再调用ngx_init_cycle方法时就可以把旧的ngx_cycle_t对象
	传进去，而这时old_cycle对象就会保存这个前期的ngx_cycle_t对象 */
    ngx_cycle_t              *old_cycle;

	/* 配置文件相对于安装目录的路径名称 */
    ngx_str_t                 conf_file;
	/* Nginx处理配置文件时需要特殊处理的在命令行携带的参数，一般是-g选项携带的参数 */
    ngx_str_t                 conf_param;
	/* Nginx配置文件所在目录的路径 */
    ngx_str_t                 conf_prefix;
	/* Nginx安装目录的路径 */
    ngx_str_t                 prefix;
	/* 用于进程间同步的文件锁名称 */
    ngx_str_t                 lock_file;
	/* 使用gethostname系统调用得到的主机名 */
    ngx_str_t                 hostname;
};


typedef struct {
    ngx_flag_t                daemon;                   // 守护进程标志位
    ngx_flag_t                master;                   // master进程标志位

    ngx_msec_t                timer_resolution;
    ngx_msec_t                shutdown_timeout;

    ngx_int_t                 worker_processes;         // worker进程标志位
    ngx_int_t                 debug_points;

    ngx_int_t                 rlimit_nofile;
    off_t                     rlimit_core;

    int                       priority;

    ngx_uint_t                cpu_affinity_auto;
    ngx_uint_t                cpu_affinity_n;
    ngx_cpuset_t             *cpu_affinity;

    char                     *username;
    ngx_uid_t                 user;
    ngx_gid_t                 group;

    ngx_str_t                 working_directory;
    ngx_str_t                 lock_file;

    ngx_str_t                 pid;
    ngx_str_t                 oldpid;

    ngx_array_t               env;
    char                    **environment;

    ngx_uint_t                transparent;  /* unsigned  transparent:1; */
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
ngx_cpuset_t *ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);
void ngx_set_shutdown_timer(ngx_cycle_t *cycle);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_dump_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
