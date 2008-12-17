/*
 * Copyright IBM Corporation. 2008
 *
 * Author:	Sudhir Kumar <skumar@linux.vnet.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description: This file contains the test code for testing libcgroup apis.
 */

#include "libcgrouptest.h"

int main(int argc, char *argv[])
{
	int fs_mounted, retval;
	struct cgroup *cgroup1, *cgroup2, *cgroup3, *nullcgroup = NULL;
	char controller_name[FILENAME_MAX], control_file[FILENAME_MAX],
		path_group[FILENAME_MAX], path_control_file[FILENAME_MAX];
	char mountpoint[FILENAME_MAX], tasksfile[FILENAME_MAX], group[FILENAME_MAX];

	/* Hardcode second mountpoint for now. Will update soon */
	char mountpoint2[FILENAME_MAX] = "/dev/cgroup_controllers-2";
	char tasksfile2[FILENAME_MAX];

	if ((argc < 3) || (atoi(argv[1]) < 0)) {
		printf("ERROR: Wrong no of parameters recieved from script\n");
		printf("Exiting the libcgroup testset\n");
		exit(1);
	}
	fs_mounted = atoi(argv[1]);
	strcpy(mountpoint, argv[2]);
	dbg("C:DBG: fs_mounted as recieved from script=%d\n", fs_mounted);
	dbg("C:DBG: mountpoint as recieved from script=%s\n", mountpoint);

	/*
	 * get the list of supported controllers
	 */
	get_controllers("cpu", &cpu);
	get_controllers("memory", &memory);
	if (cpu == 0 && memory == 0) {
		fprintf(stderr, "controllers are not enabled in kernel\n");
		fprintf(stderr, "Exiting the libcgroup testcases......\n");
		exit(1);
	}

	/* Set default control permissions */
	control_uid = 0;
	control_gid = 0;
	tasks_uid = 0;
	tasks_gid = 0;

	/*
	 * Testsets: Testcases are broadly devided into 3 categories based on
	 * filesystem(fs) mount scenario. fs not mounted, fs mounted, fs multi
	 * mounted. Call different apis in these different scenarios.
	 */

	switch (fs_mounted) {

	case FS_NOT_MOUNTED:

		/*
		 * Test01: call cgroup_init() and check return values
		 * Exp outcome: error ECGROUPNOTMOUNTED
		 */

		retval = cgroup_init();
		if (retval == ECGROUPNOTMOUNTED)
			printf("Test[0:%2d]\tPASS: cgroup_init() retval= %d:\n",\
								 ++i, retval);
		else
			printf("Test[0:%2d]\tFAIL: cgroup_init() retval= %d:\n",\
								 ++i, retval);

		/*
		 * Test02: call cgroup_attach_task() with null group
		 * Exp outcome: error non zero return value
		 */
		retval = cgroup_attach_task(nullcgroup);
		if (retval != 0)
			printf("Test[0:%2d]\tPASS: cgroup_attach_task() ret: %d\n",\
								 ++i, retval);
		else
			printf("Test[0:%2d]\tFAIL: cgroup_attach_task() ret: %d\n",\
								 ++i, retval);

		/*
		 * Test03: Create a valid cgroup and check all return values
		 * Exp outcome: no error. 0 return value
		 */

		strncpy(group, "group1", sizeof(group));
		retval = set_controller(MEMORY, controller_name, control_file);
		strncpy(val_string, "40960000", sizeof(val_string));

		if (retval)
			fprintf(stderr, "Setting controller failled\n");

		cgroup1 = new_cgroup(group, controller_name,
						 control_file, STRING);

		/*
		 * Test04: Then Call cgroup_create_cgroup() with this valid group
		 * Exp outcome: non zero return value
		 */
		retval = cgroup_create_cgroup(cgroup1, 1);
		if (retval)
			printf("Test[0:%2d]\tPASS: cgroup_create_cgroup() retval=%d\n", ++i, retval);
		else
			printf("Test[0:%2d]\tFAIL: cgroup_create_cgroup() retval=%d\n", ++i, retval);

		/*
		 * Test05: delete cgroup
		 * Exp outcome: non zero return value but what ?
		 */
		retval = cgroup_delete_cgroup(cgroup1, 1);
		if (retval)
			printf("Test[0:%2d]\tPASS: cgroup_delete_cgroup() retval=%d\n", ++i, retval);
		else
			printf("Test[0:%2d]\tFAIL: cgroup_delete_cgroup() retval=%d\n", ++i, retval);

		/*
		 * Test06: Check if cgroup_create_cgroup() handles a NULL cgroup
		 * Exp outcome: error ECGINVAL
		 */
		retval = cgroup_create_cgroup(nullcgroup, 1);
		if (retval)
			printf("Test[0:%2d]\tPASS: cgroup_create_cgroup() nullcgroup handled\n", ++i);
		else
			printf("Test[0:%2d]\tFAIL: cgroup_create_cgroup() nullcgroup not handled\n", ++i);

		/*
		 * Test07: delete nullcgroup
		 */
		retval = cgroup_delete_cgroup(nullcgroup, 1);
		if (retval)
			printf("Test[0:%2d]\tPASS: cgroup_delete_cgroup() nullcgroup handled\n", ++i);
		else
			printf("Test[0:%2d]\tFAIL: cgroup_delete_cgroup() Unable to handle nullcgroup\n", ++i);

		cgroup_free(&nullcgroup);
		cgroup_free(&cgroup1);

		break;

	case FS_MOUNTED:

		/* Do a sanity check if cgroup fs is mounted */
		if (check_fsmounted()) {
			printf("Sanity check fails. cgroup fs not mounted\n");
			printf("Exiting without running this set of tests\n");
			exit(1);
		}

		/*
		 * Test01: call cgroup_attach_task() with null group
		 * without calling cgroup_inti(). We can check other apis too.
		 * Exp outcome: error ECGROUPNOTINITIALIZED
		 */
		retval = cgroup_attach_task(nullcgroup);
		if (retval == ECGROUPNOTINITIALIZED)
			printf("Test[1:%2d]\tPASS: cgroup_attach_task() ret: %d\n",\
								 ++i, retval);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_attach_task() ret: %d\n",\
								 ++i, retval);


		/*
		 * Test02: call cgroup_init() and check return values
		 * Exp outcome:  no error. return value 0
		 */

		retval = cgroup_init();
		if (retval == 0)
			printf("Test[1:%2d]\tPASS: cgroup_init() retval= %d:\n",\
								 ++i, retval);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_init() retval= %d:\n",\
								 ++i, retval);

		/*
		 * Test03: Call cgroup_attach_task() with null group and check if
		 * return values are correct. If yes check if task exists in
		 * root group tasks file
		 * TODO: This test needs some modification in script
		 * Exp outcome: current task should be attached to root group
		 */
		retval = cgroup_attach_task(nullcgroup);
		if (retval == 0) {
			strncpy(tasksfile, mountpoint, sizeof(mountpoint));
			strcat(tasksfile, "/tasks");
			if (check_task(tasksfile))
				return -1;
		} else {
			printf("Test[1:%2d]\tFAIL: cgroup_attach_task() ret: %d\n",\
								 ++i, retval);
		}

		/*
		 * Test04: Call cgroup_attach_task_pid() with null group
		 * and invalid pid
		 * Exp outcome: error
		 */
		retval = cgroup_attach_task_pid(nullcgroup, -1);
		if (retval != 0)
			printf("Test[1:%2d]\tPASS: cgroup_attach_task_pid() ret: %d\n",\
								 ++i, retval);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_attach_task_pid() ret: %d\n",\
								 ++i, retval);

		/*
		 * Test05: Create a valid cgroup structure
		 * Exp outcome: no error. 0 return value
		 */
		strncpy(group, "group1", sizeof(group));
		retval = set_controller(MEMORY, controller_name, control_file);
		strncpy(val_string, "40960000", sizeof(val_string));

		if (retval) {
			retval = set_controller(CPU, controller_name,
								control_file);
			strncpy(val_string, "2048", sizeof(val_string));
			if (retval) {
				fprintf(stderr, "Failed to set any controllers "
					"Tests dependent on this structure will"
					" fail\n");
			}
		}

		cgroup1 = new_cgroup(group, controller_name,
						 control_file, STRING);

		/*
		 * Test06: Then Call cgroup_create_cgroup() with this valid group
		 * Exp outcome: zero return value
		 */
		retval = cgroup_create_cgroup(cgroup1, 1);
		if (!retval) {
			/* Check if the group exists in the dir tree */
			strncpy(path_group, mountpoint, sizeof(mountpoint));
			strncat(path_group, "/group1", sizeof("group1"));
			if (group_exist(path_group) == 0)
				printf("Test[1:%2d]\tPASS: cgroup_create_cgroup() retval=%d\n",
										 ++i, retval);
		} else
			printf("Test[1:%2d]\tFAIL: cgroup_create_cgroup() retval=%d\n", ++i, retval);

		/*
		 * Create another valid cgroup structure
		 * Exp outcome: no error. 0 return value
		 */
		strncpy(group, "group1", sizeof(group));
		retval = set_controller(MEMORY, controller_name, control_file);
		strncpy(val_string, "81920000", sizeof(val_string));

		if (retval)
			fprintf(stderr, "Setting controller failled\n");

		cgroup2 = new_cgroup(group, controller_name,
						 control_file, STRING);

		/*
		 * Test07: modify cgroup
		 * Exp outcome: zero return value
		 */
		strncpy(path_control_file, mountpoint, sizeof(mountpoint));
		strncat(path_control_file, "/group1", sizeof("group1"));
		strncat(path_control_file, "/", sizeof("/"));
		strncat(path_control_file, control_file, sizeof(control_file));

		retval = cgroup_modify_cgroup(cgroup2);
		/* Check if the values are changed */
		if (!retval && !group_modified(path_control_file, STRING))
			printf("Test[1:%2d]\tPASS: cgroup_modify_cgroup() retval=%d\n", ++i, retval);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_modify_cgroup() retval=%d\n", ++i, retval);

		/*
		 * Test08: modify cgroup with the same cgroup
		 * Exp outcome: zero return value
		 */

		retval = cgroup_modify_cgroup(cgroup1);
		/* Check if the values are changed */
		if (!retval && !group_modified(path_control_file, STRING))
			printf("Test[1:%2d]\tPASS: cgroup_modify_cgroup() retval=%d\n", ++i, retval);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_modify_cgroup() retval=%d\n", ++i, retval);

		/*
		 * Test09: modify cgroup with the null cgroup
		 * Exp outcome: zero return value. root group unchanged.
		 */

		strncpy(path_control_file, mountpoint, sizeof(mountpoint));
		strncat(path_control_file, "/", sizeof("/"));
		strncat(path_control_file, control_file, sizeof(control_file));

		retval = cgroup_modify_cgroup(nullcgroup);
		/* Check if the values are changed */
		if (!retval && !group_modified(path_control_file, STRING))
			printf("Test[1:%2d]\tPASS: cgroup_modify_cgroup() retval=%d\n", ++i, retval);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_modify_cgroup() retval=%d\n", ++i, retval);

		/*
		 * Create another valid cgroup structure with diff controller
		 * to modify the existing group
		 * Exp outcome: no error. 0 return value
		 */
		val_int64 = 2048;
		strncpy(group, "group1", sizeof(group));
		retval = set_controller(CPU, controller_name, control_file);
		if (retval)
			fprintf(stderr, "Setting controller failled\n");

		cgroup3 = new_cgroup(group, controller_name,
						 control_file, INT64);

		/*
		 * Test10: modify existing group with this cgroup
		 * Exp outcome: zero return value
		 */
		strncpy(path_control_file, mountpoint, sizeof(mountpoint));
		strncat(path_control_file, "/group1", sizeof("group2"));
		strncat(path_control_file, "/", sizeof("/"));
		strncat(path_control_file, control_file, sizeof(control_file));

		strncpy(val_string, "2048", sizeof(val_string));

		retval = cgroup_modify_cgroup(cgroup3);
		/* Check if the values are changed */
		if (!retval && !group_modified(path_control_file, STRING))
			printf("Test[1:%2d]\tPASS: cgroup_modify_cgroup() retval=%d\n", ++i, retval);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_modify_cgroup() retval=%d\n", ++i, retval);


		/*
		 * Test11: delete cgroup
		 * Exp outcome: zero return value
		 */
		retval = cgroup_delete_cgroup(cgroup1, 1);
		if (!retval) {
			/* Check if the group is deleted from the dir tree */
			strncpy(path_group, mountpoint, sizeof(mountpoint));
			strncat(path_group, "/group1", sizeof("group1"));
			if (group_exist(path_group) == -1)
				printf("Test[1:%2d]\tPASS: cgroup_delete_cgroup() retval=%d\n",
										 ++i, retval);
		} else
			printf("Test[1:%2d]\tFAIL: cgroup_delete_cgroup() retval=%d\n", ++i, retval);

		/*
		 * Test12: Check if cgroup_create_cgroup() handles a NULL cgroup
		 * Exp outcome: error ECGINVAL
		 */
		retval = cgroup_create_cgroup(nullcgroup, 1);
		if (retval)
			printf("Test[1:%2d]\tPASS: cgroup_create_cgroup() nullcgroup handled\n", ++i);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_create_cgroup() nullcgroup not handled\n", ++i);

		/*
		 * Test13: delete nullcgroup
		 */
		retval = cgroup_delete_cgroup(nullcgroup, 1);
		if (retval)
			printf("Test[1:%2d]\tPASS: cgroup_delete_cgroup() nullcgroup handled\n", ++i);
		else
			printf("Test[1:%2d]\tFAIL: cgroup_delete_cgroup() Unable to handle nullcgroup\n", ++i);

		cgroup_free(&nullcgroup);
		cgroup_free(&cgroup1);

		break;

	case FS_MULTI_MOUNTED:
		/*
		 * Test01: call apis and check return values
		 * Exp outcome:
		 */

		/*
		 * Scenario 1: cgroup fs is multi mounted
		 * Exp outcome: no error. 0 return value
		 */

		retval = cgroup_init();
		if (retval == 0)
			printf("Test[2:%2d]\tPASS: cgroup_init() retval= %d:\n",\
								 ++i, retval);
		else
			printf("Test[2:%2d]\tFAIL: cgroup_init() retval= %d:\n",\
								 ++i, retval);

		/*
		 * Test02: Call cgroup_attach_task() with null group and check if
		 * return values are correct. If yes check if task exists in
		 * root group tasks file for each controller
		 * TODO: This test needs some modification in script
		 * Exp outcome: current task should be attached to root groups
		 */
		retval = cgroup_attach_task(nullcgroup);
		if (retval == 0) {
			strncpy(tasksfile, mountpoint, sizeof(mountpoint));
			strcat(tasksfile, "/tasks");
			strncpy(tasksfile2, mountpoint2, sizeof(mountpoint));
			strcat(tasksfile2, "/tasks");
			if (check_task(tasksfile) || i-- && check_task(tasksfile2))
				return -1;
		} else {
			printf("Test[2:%2d]\tFAIL: cgroup_attach_task() ret: %d\n",\
						 ++i, retval);
		}
		break;

	default:
		fprintf(stderr, "ERROR: Wrong parameters recieved from script. \
						Exiting tests\n");
		exit(1);
		break;
	}
	return 0;
}

void get_controllers(char *name, int *exist)
{
	int hierarchy, num_cgroups, enabled;
	FILE *fd;
	char subsys_name[FILENAME_MAX];
	fd = fopen("/proc/cgroups", "r");
	if (!fd)
		return;

	while (!feof(fd)) {
		fscanf(fd, "%s, %d, %d, %d", subsys_name,
					 &hierarchy, &num_cgroups, &enabled);
		if (strncmp(name, subsys_name, sizeof(*name)) == 0)
			*exist = 1;
	}
}

static int group_exist(char *path_group)
{
	int ret;
	ret = open(path_group, O_DIRECTORY);
	if (ret == -1)
		return ret;
	return 0;
}

static int set_controller(int controller, char *controller_name,
							 char *control_file)
{
	switch (controller) {
	case MEMORY:
		if (memory == 0)
			return 1;

		strncpy(controller_name, "memory", FILENAME_MAX);
		strncpy(control_file, "memory.limit_in_bytes", FILENAME_MAX);
		return 0;
		break;

	case CPU:
		if (cpu == 0)
			return 1;

		strncpy(controller_name, "cpu", FILENAME_MAX);
		strncpy(control_file, "cpu.shares", FILENAME_MAX);
		return 0;
		break;
		/* Future controllers can be added here */

	default:
		return 1;
		break;
	}
}

static int group_modified(char *path_control_file, int value_type)
{
	bool bool_val;
	int64_t int64_val;
	u_int64_t uint64_val;
	char string_val[FILENAME_MAX]; /* Doubt: what should be the size ? */
	FILE *fd;

	fd = fopen(path_control_file, "r");
	if (!fd) {
		fprintf(stderr, "Error in opening %s\n", path_control_file);
		fprintf(stderr, "Skipping modified values check....\n");
		return 1;
	}

	switch (value_type) {

	case BOOL:
		fscanf(fd, "%d", &bool_val);
		if (bool_val == val_bool)
			return 0;
		break;
	case INT64:
		fscanf(fd, "%lld", &int64_val);
		if (int64_val == val_int64)
			return 0;
		break;
	case UINT64:
		fscanf(fd, "%llu", &uint64_val);
		if (uint64_val == val_uint64)
			return 0;
		break;
	case STRING:
		fscanf(fd, "%s", string_val);
		if (!strncmp(string_val, val_string, strlen(string_val)))
			return 0;
		break;
	default:
		fprintf(stderr, "Wrong value_type passed in group_modified()\n");
		fprintf(stderr, "Skipping modified values check....\n");
		return 0;	/* Can not report test result as failure */
		break;
	}
	return 1;
}

struct cgroup *new_cgroup(char *group, char *controller_name,
				 char *control_file, int value_type)
{
	int retval;
	struct cgroup *newcgroup;
	struct cgroup_controller *newcontroller;
	newcgroup = cgroup_new_cgroup(group);

	if (newcgroup) {
		retval = cgroup_set_uid_gid(newcgroup, tasks_uid, tasks_gid,
						control_uid, control_gid);

		if (retval) {
			printf("Test[1:%2d]\tFAIL: cgroup_set_uid_gid()\n",
								++i);
		}

		newcontroller = cgroup_add_controller(newcgroup, controller_name);
		if (newcontroller) {
			switch (value_type) {

			case BOOL:
				retval = cgroup_add_value_bool(newcontroller,
						 control_file, val_bool);
				break;
			case INT64:
				retval = cgroup_add_value_int64(newcontroller,
						 control_file, val_int64);
				break;
			case UINT64:
				retval = cgroup_add_value_uint64(newcontroller,
						 control_file, val_uint64);
				break;
			case STRING:
				retval = cgroup_add_value_string(newcontroller,
						 control_file, val_string);
				break;
			default:
				printf("ERROR: wrong value in new_cgroup()\n");
				return NULL;
				break;
			}

			if (!retval) {
				printf("Test[1:%2d]\tPASS: cgroup_new_cgroup() success\n", ++i);
			} else {
				printf("Test[1:%2d]\tFAIL: cgroup_add_value_string()\n", ++i);
				return NULL;
			}
		 } else {
			printf("Test[1:%2d]\tFAIL: cgroup_add_controller()\n", ++i);
			return NULL;
		}
	} else {
		printf("Test[1:%2d]\tFAIL: cgroup_new_cgroup() fails\n", ++i);
		return NULL;
	}
	return newcgroup;
}

int check_fsmounted()
{
	struct mntent *entry, *tmp_entry;
	/* Need a better mechanism to decide memory allocation size here */
	char entry_buffer[FILENAME_MAX * 4];
	FILE *proc_file;

	tmp_entry = (struct mntent *) malloc(sizeof(struct mntent));
	if (!tmp_entry) {
		perror("Error: failled to mallloc for mntent\n");
		return 1;
	}

	proc_file = fopen("/proc/mounts", "r");
	if (!proc_file) {
		printf("Error in opening /proc/mounts.\n");
		return EIO;
	}
	while ((entry = getmntent_r(proc_file, tmp_entry, entry_buffer, FILENAME_MAX*4)) != NULL) {
		if (!strncmp(entry->mnt_type, "cgroup", strlen("cgroup"))) {
			printf("sanity check pass.... %s\n", entry->mnt_type);
			return 0;
		}
	}
	return 1;
}

static int check_task(char *tasksfile)
{
	FILE *file;
	pid_t curr_tid, tid;
	int pass = 0;

	file = fopen(tasksfile, "r");
	if (!file) {
		printf("ERROR: in opening %s\n", tasksfile);
		return -1;
	}

	curr_tid = cgrouptest_gettid();
	while (!feof(file)) {
		fscanf(file, "%u", &tid);
		if (tid == curr_tid) {
			pass = 1;
			break;
		}
	}
	if (pass)
		printf("Test[2:%2d]\tPASS: Task found in %s\n", ++i, tasksfile);
	else
		printf("Test[2:%2d]\tFAIL: Task not found in %s\n", ++i, tasksfile);

	return 0;
}
