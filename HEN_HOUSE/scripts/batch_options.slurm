
###############################################################################
#
#  EGSnrc scheduler options for Slurm
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Marc-Andre Renaud, 2016
#
#  Contributors:
#
###############################################################################
#
#  Various definitions for job sumbission using Slurm. This file gets sourced
#  from $HEN_HOUSE/scripts/run_user_code_batch if the environment variable
#  EGS_BATCH_OPTIONS is set to slurm or 'batch=slurm' is passed as argument.
#
###############################################################################


# The name of the command used to submit jobs to the queue
batch_command=sbatch

generic_bo="-n1"

# Flag to specify where the output from this job should go.
# If it is not empty, the job output will be send to input_file.eo
# where input_file is the name of the input file or input_file_wX.eo
# where X is the job number in the case of a parallel run.
# If there is no such option (e.g. `at'), just leave it empty.
#
output_bo="-o"

# Flag to specify how to name the request.
# Giving the job request a name is handy as it helps us find it in a long
# list of running or waiting jobs. The job request will be normally
# named usercode_inputfile where usercode is the name of the user code
# and input file the name of the input file.
# If there is no such option (e.g. `at'), just leave it empty.
#
rname_bo="-J"

# Should the batch submission script sleep between jobs ?
# (only relevant for parallel job submission)
# If the following is left empty, the script will not sleep.
# Sleeping time > 0 is only necessary for PBS because PBS is
# extremely fast in sumbitting jobs but gets confused if many
# jobs are submitted quickly. A sleep time of 1 second is sufficient
# on out system,
#
batch_sleep_time=1

# Some queueing systems (e.g. PBS) have a limitation on the
# maximum length of a job name (e.g. for PBS this is 15).
# The following variable specifies such a limit and the
# job name is truncated to that length if necessary in the
# job submission script.
#
batch_mnl=100

# Set the Slurm partition names used in your cluster.
# Change these if you have defined different queues.
#
short_queue="-p short"
long_queue="-p long"
user1_queue="-p user1"
user2_queue="-p user2"
user3_queue="-p user3"
