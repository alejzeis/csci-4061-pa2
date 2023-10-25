# CSCI4061 PA1

**Project Group**: 133

**Group Members**:
- Alejandro Zeise (zeise035)
- Charlie Youngren (youn2835)

**Device Tested On**: Personal Laptop, running Fedora 38 (Linux 6.4.15, GCC 13.2.1)

**Existing File Changes**:
- None 

**Contributions:**
- Alejandro: ```leaf_process.c```, ```nonleaf_process.c```
- Charlie: ```root_process.c```

## Plan
1. The root process parses it's arguments, creates a pipe and forks itself. The child will call exec() on the first nonleaf_process.
2. Each nonleaf_process will do the following:
```
int pipeToParent[2]
int current_pipe[2];
pid_t pids[];
foreach element in directory:
    current_pipe = create_new_pipe()
    if isFile(element):
        pids <- fork()
        if child:
            // Pass pipe FD, file path
            exec(leaf process)
        else:
            close(current_pipe.write_end)

            data = read(new_pipe)
            write(pipeToParent.write_end, data)
    elseif isDirectory(element):
        // Same thing as above, instead exec a nonleaf_process

// Wait for all children to quit
foreach pid in pids:
    waitforpid(pid)
```
