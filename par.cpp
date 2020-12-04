/* Copyright 2019 Tjark Weber <tjark.weber@it.uu.se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <thread>
#include <cassert>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>

using namespace std;

enum LOGIC
  {
    LOGIC_UNKNOWN,
    LOGIC_AUFLIRA,
    LOGIC_AUFNIRA,
    LOGIC_BV,
    LOGIC_LRA,
    LOGIC_NRA,
    LOGIC_QF_ABV,
    LOGIC_QF_BV,
    LOGIC_QF_BVFP,
    LOGIC_QF_FP,
    LOGIC_QF_IDL,
    LOGIC_QF_LIA,
    LOGIC_QF_LIRA,
    LOGIC_QF_LRA,
    LOGIC_QF_NIA,
    LOGIC_QF_NRA,
    LOGIC_QF_UF,
    LOGIC_QF_UFNRA,
    LOGIC_UF,
    LOGIC_UFBV,
    LOGIC_UFIDL,
    LOGIC_UFLIA,
    LOGIC_UFNIA
  };

map<string,LOGIC> logic_of_string = {
  { "AUFLIRA", LOGIC_AUFLIRA },
  { "AUFNIRA", LOGIC_AUFNIRA },
  { "BV", LOGIC_BV },
  { "LRA", LOGIC_LRA },
  { "NRA", LOGIC_NRA },
  { "QF_ABV", LOGIC_QF_ABV },
  { "QF_BV", LOGIC_QF_BV },
  { "QF_BVFP", LOGIC_QF_BVFP },
  { "QF_FP", LOGIC_QF_FP },
  { "QF_IDL", LOGIC_QF_IDL },
  { "QF_LIA", LOGIC_QF_LIA },
  { "QF_LIRA", LOGIC_QF_LIRA },
  { "QF_LRA", LOGIC_QF_LRA },
  { "QF_NIA", LOGIC_QF_NIA },
  { "QF_NRA", LOGIC_QF_NRA },
  { "QF_UF", LOGIC_QF_UF },
  { "QF_UFNRA", LOGIC_QF_UFNRA },
  { "UF", LOGIC_UF },
  { "UFBV", LOGIC_UFBV },
  { "UFIDL", LOGIC_UFIDL },
  { "UFLIA", LOGIC_UFLIA },
  { "UFNIA", LOGIC_UFNIA }
};

LOGIC read_logic(const char* benchmark)
{
  // we search for a line of the form "[[:space:]]*(set-logic[[:space:]].*).*"

  FILE *file = fopen(benchmark, "r");
  if (!file)
    {
      return LOGIC_UNKNOWN;
    }

  LOGIC result;

  while (true)
    {
      char *line = NULL;
      size_t n;
      if (getline(&line, &n, file) == -1)
        {
          fclose(file);
          return LOGIC_UNKNOWN;
        }
      assert(line);

      char *str = line;
      // skip whitespace
      while (isspace(*str))
        {
          ++str;
        }
      // do we find (set-logic[[:space:]] ? otherwise continue with next line
      if (strncmp(str, "(set-logic", strlen("(set_logic")) != 0)
        {
          free(line);
          continue;
        }
      str += strlen("(set-logic");
      if (!isspace(*str))
        {
          free(line);
          continue;
        }
      ++str;

      // determine logic name by searching for ')'
      size_t logic_size = 0;
      while (str[logic_size] && str[logic_size] != ')')
        {
          ++logic_size;
        }
      if (str[logic_size] != ')')
        {
          result = LOGIC_UNKNOWN;
        }
      else
        {
          string logic_name(str, logic_size);
          try
            {
              result = logic_of_string.at(logic_name);
            }
          catch (const out_of_range& oor)
            {
              result = LOGIC_UNKNOWN;
            }
        }
      free(line);
      break;
    }

  fclose(file);
  return result;
}

typedef list<string> SOLVERS;

// based on the SMT-COMP 2018 main track results (excluding any solver
// in any division where it produced an error); a greedy algorithm was
// used to choose (for each logic) a combination of solvers that
// maximizes the number of benchmarks solved (and minimizes the time
// for solving)
map<LOGIC,SOLVERS> solvers_of_logic = {
  { LOGIC_AUFLIRA, { "19792", "19789" } },
  { LOGIC_AUFNIRA, { "19775", "19792" } },
  { LOGIC_BV, { "19775", "19771", "19792" } },
  { LOGIC_LRA, { "19792", "19775" } },
  { LOGIC_NRA, { "19792", "19775" } },
  { LOGIC_QF_ABV, { "19771", "19791", "19775" } },
  { LOGIC_QF_BV, { "19771", "19775", "19791" } },
  { LOGIC_QF_BVFP, { "19775", "19772", "19792" } },
  { LOGIC_QF_FP, { "19775", "19772", "19792" } },
  { LOGIC_QF_IDL, { "19791", "19792", "19775", "19789" } },
  { LOGIC_QF_LIA, { "19784", "19791", "19792" } },
  { LOGIC_QF_LIRA, { "19792", "19783" } },
  { LOGIC_QF_LRA, { "19784", "19775", "19781", "19789" } },
  { LOGIC_QF_NIA, { "19791", "19775", "19770", "19792" } },
  { LOGIC_QF_NRA, { "19791", "19775", "19792" } },
  { LOGIC_QF_UF, { "19791", "19792" } },
  { LOGIC_QF_UFNRA, { "19791", "19792" } },
  { LOGIC_UF, { "19775", "19789", "19792" } },
  { LOGIC_UFBV, { "19792", "19775" } },
  { LOGIC_UFIDL, { "19792", "19775" } },
  { LOGIC_UFLIA, { "19775", "19792", "19789" } },
  { LOGIC_UFNIA, { "19792", "19775" } }
};

////////////////////////////////////////////////////////////////////////////////

// returns true if the first size bytes of buf are equal to [[:space:]]* cmp [[:space:]]*
bool compare_skip_space (ssize_t size, const char *buf, const char* cmp)
{
  while (size > 0 && isspace(*buf))
    {
      ++buf;
      --size;
    }
  while (*cmp)
    {
      if (size < 1 || *buf != *cmp)
        {
          return false;
        }
      ++buf;
      --size;
      ++cmp;
    }
  while (size > 0)
    {
      if (!isspace(*buf))
        {
          return false;
        }
      ++buf;
      --size;
    }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

//#define VERBOSE

////////////////////////////////////////////////////////////////////////////////

string result = "unknown";

void kill_all()
{
  cout << result << endl;
  kill(0, SIGINT); // SIGINT seems to work more reliably than SIGTERM (?)
                   // FIXME: sometimes, not all child processes terminate
  pause(); // do not return
}

#define EXIT_ERROR 1

int main(int argc, char* argv[])
{
  setsid(); // cf. kill_all, which kills the entire process group;
            // so we want this process to be the group's leader

  if (argc!=4)
    {
      cout << "Usage: " << argv[0] << " SOLVERS SOLUTIONS BENCHMARK" << endl
           << endl
           << "  SOLVERS is the (maximum) number of solvers that will be run in" << endl
           << "  parallel. Set to 0 to use number of hardware threads." << endl
           << endl
           << "  SOLUTIONS is the number of solvers that need to agree on the" << endl
           << "  answer before it is reported." << endl
           << endl
           << "  BENCHMARK is the name of the benchmark file." << endl;
      return EXIT_ERROR;
    }

  // number of threads
  unsigned int threads = atoi(argv[1]);
  if (!threads)
    {
      threads = thread::hardware_concurrency();
    }
  if (!threads)
    {
      cout << "ERROR: cannot determine number of hardware threads." << endl;
      return EXIT_ERROR;
    }

  // number of solutions
  unsigned int solutions = atoi(argv[2]);
  if (!solutions)
    {
      cout << "ERROR: SOLUTIONS must be a positive integer" << endl;
      return EXIT_ERROR;
    }

  // real benchmark name
  char *benchmark = realpath(argv[3], NULL); // should be free'd later
  if (!benchmark)
    {
      cout << "ERROR: cannot determine real benchmark path." << endl;
      return EXIT_ERROR;
    }

  // change to program directory
  char *dir = realpath(argv[0], NULL); // should be free'd later
  if (!dir)
    {
      cout << "ERROR: cannot determine full program path." << endl;
      return EXIT_ERROR;
    }
  dir = dirname(dir);
  if (!dir)
    {
      cout << "ERROR: cannot determine program directory." << endl;
      return EXIT_ERROR;
    }
  if (chdir(dir) != 0)
    {
      cout << "ERROR: cannot change to program directory." << endl;
      return EXIT_ERROR;
    }

  // benchmark logic
  LOGIC logic = read_logic(benchmark);
  if (logic==LOGIC_UNKNOWN)
    {
#ifdef VERBOSE
      cout << "ERROR: logic not supported." << endl; // print 'not supported' rather than 'unknown'
                                                     // ('unknown' would confuse the StarExec post-processor)
#endif
      cout << "unknown" << endl;
      return EXIT_ERROR;
    }

  // launch solvers
  SOLVERS solvers = solvers_of_logic.at(logic);
  threads = std::min(threads, (unsigned int)solvers.size());
#ifdef VERBOSE
  if (solutions > threads)
    {
      cout << "Warning: SOLUTIONS > SOLVERS (no result possible)." << endl;
    }
#endif

  FILE *child[threads];
  int child_fd[threads];

  auto it = begin(solvers);
  for (unsigned int i=0; i<threads; ++i)
    {
      assert(it != end(solvers));

      string cmd = "cd '../solvers/" + *it + "/bin' && ./starexec_run_default '" + benchmark + "' 2>/dev/null";

      child[i] = popen(cmd.c_str(), "r"); // should be pclose'd later
      // (but we don't want to wait for children)

      if (child[i])
        {
          child_fd[i] = fileno(child[i]);
        }
#ifdef VERBOSE
      else
        {
          cout << "Warning: cannot launch solver " << *it << "." << endl;
        }
#endif

      ++it;
    }

  unsigned int sat_responses = 0;
  unsigned int unsat_responses = 0;

  // read response (sat/unsat) from solvers
  while (1)
    {
      bool has_children = false;
      fd_set read_fds;
      FD_ZERO(&read_fds);
      for (unsigned int i=0; i<threads; ++i)
        {
          if (child[i])
            {
              has_children = true;
              FD_SET(child_fd[i], &read_fds);
            }
        }

      if (!has_children)
        {
          kill_all();
        }

      while (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) < 1)
        {
#ifdef VERBOSE
          cout << "Warning: re-selecting." << endl;
#endif
        }

      for (unsigned int i=0; i<threads; ++i)
        {
          if (child[i] && FD_ISSET(child_fd[i], &read_fds))
            {
              char buffer[1024];
              ssize_t size = read(child_fd[i], buffer, sizeof(buffer));

              // process each line in buffer separately
              ssize_t line_start = 0;
              while (line_start < size)
                {
                  ssize_t line_end = line_start;
                  while (line_end < size && buffer[line_end] != '\n')
                    {
                      ++line_end;
                    }
                  if (line_end < size)
                    {
                      ++line_end; // include '\n' in line
                    }

                  ssize_t line_size = line_end - line_start;
                  assert(line_size > 0);

                  if (compare_skip_space(line_size, buffer+line_start, "sat"))
                    {
                      ++sat_responses;
                    }
                  else if (compare_skip_space(line_size, buffer+line_start, "unsat"))
                    {
                      ++unsat_responses;
                    }
                  else if (compare_skip_space(line_size, buffer+line_start, "success")
                           || compare_skip_space(line_size, buffer+line_start, "unknown")
                           || compare_skip_space(line_size, buffer+line_start, ""))
                    {
                      // ignore "success" responses, "unknown" responses and empty lines
                    }
                  else
                    {
                      child[i] = NULL;
#ifdef VERBOSE
                      auto it = begin(solvers);
                      advance(it, i);
                      cout << "Warning: unexpected response from solver " << i << " (" << *it << "): '"
                           << string(buffer+line_start, line_size) << "'" << endl;
#endif
                    }

                  if (sat_responses && unsat_responses)
                    {
#ifdef VERBOSE
                      cout << "Warning: conflicting solver responses." << endl;
#endif
                      kill_all();
                    }
                  else if (sat_responses >= solutions)
                    {
                      result = "sat";
                      kill_all();
                    }
                  else if (unsat_responses >= solutions)
                    {
                      result = "unsat";
                      kill_all();
                    }

                  line_start = line_end;
                }
            }
        }
    }
}
