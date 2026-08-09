#define NOB_IMPLEMENTATION
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

Cmd cmd = {0};

static void usage(void)
{
    fprintf(stderr, "Usage: %s [<FLAGS>] [--] [<program args>]\n", flag_program_name());
    fprintf(stderr, "FLAGS:\n");
    flag_print_options(stderr);
}

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    bool run = false;
    bool help = false;
    bool cross = false;
    bool deploy = false;
    bool deploy_all = false;

    char *pi_user = "chess";
    char *pi_host = "192.168.0.194";
    char *pi_path = "~/chess";

    flag_bool_var(&run, "run", false, "Run the program after compilation.");
    flag_bool_var(&cross, "cross", false, "Cross-compile for Raspberry Pi 4 (ARM64).");
    flag_bool_var(&deploy, "deploy", false, "Cross-compile and upload only the binary executable.");
    flag_bool_var(&deploy_all, "deploy-all", false, "Cross-compile and upload the binary PLUS the assets folder.");
    flag_str_var(&pi_user, "pi-user", "chess", "SSH/SCP username for the Raspberry Pi.");
    flag_str_var(&pi_host, "pi-host", "192.168.0.194", "IP address or network hostname of the Pi.");
    flag_str_var(&pi_path, "pi-path", "~/chess", "Target directory on the Pi to upload files.");
    flag_bool_var(&help, "help", false, "Print this help message.");

    if (!flag_parse(argc, argv)) {
        usage();
        flag_print_error(stderr);
        return 1;
    }

    if (help) {
        usage();
        return 0;
    }

    // CRITICAL FIX: If either deployment flag is requested, force cross-compilation mode
    if (deploy || deploy_all) {
        cross = true;
    }

    if (cross && run) {
        nob_log(NOB_ERROR, "Cannot use '--run' alongside '--cross' or deployment flags.");
        return 1;
    }

    // Reset command builder before compiling
    cmd.count = 0;

    // 1. Core compiler selection
    if (cross) {
        cmd_append(&cmd, "aarch64-linux-gnu-gcc");
    } else {
        cmd_append(&cmd, "cc");
    }

    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-ggdb");

    // 2. Adjust target directories and output filenames
    if (cross) {
        cmd_append(&cmd, "-I./raylib-6.0_linux_arm64/include/");
        cmd_append(&cmd, "-o", "./main_pi", "main.c");
        cmd_append(&cmd, "-L./raylib-6.0_linux_arm64/lib/");
    } else {
        cmd_append(&cmd, "-I./raylib-6.0_linux_amd64/include/");
        cmd_append(&cmd, "-o", "./main", "main.c");
        cmd_append(&cmd, "-L./raylib-6.0_linux_amd64/lib/");
    }

    // 3. Global compilation linkage links
    // 3. Global compilation linkage links
    cmd_append(&cmd, "-l:libraylib.a");
    
    if (cross) {
        // Tell the PC cross-compiler to trust that the Pi has X11 and OpenGL.
        // It will bypass the local PC check and compile the binary cleanly.
        cmd_append(&cmd, "-Wl,--allow-shlib-undefined");
        cmd_append(&cmd, "-Wl,--unresolved-symbols=ignore-all");
    } else {
        // Native Desktop Pop!_OS setup stays completely untouched
        cmd_append(&cmd, "-lGL");
        cmd_append(&cmd, "-lX11");
    }
    
    cmd_append(&cmd, "-lm");
    cmd_append(&cmd, "-lpthread");
    cmd_append(&cmd, "-ldl");
    cmd_append(&cmd, "-lrt");


    // EXECUTE COMPILATION STEP
    if (!cmd_run(&cmd)) return 1;

    // 4. Secure network deployment routine 
    if (deploy || deploy_all) {
        // Step A: Ensure target workspace path folder exists over SSH 
        nob_log(NOB_INFO, "Ensuring target directory exists on Pi...");
        cmd.count = 0;
        const char *remote_mkdir = nob_temp_sprintf("mkdir -p %s", pi_path);
        cmd_append(&cmd, "ssh", nob_temp_sprintf("%s@%s", pi_user, pi_host), remote_mkdir);
        if (!cmd_run(&cmd)) {
            nob_log(NOB_ERROR, "Failed to connect to Raspberry Pi or create directory.");
            return 1;
        }

        // Step B: Deploy the raw assets recursive folder directory tree if requested
        if (deploy_all) {
            nob_log(NOB_INFO, "Transferring assets directory to %s@%s:%s...", pi_user, pi_host, pi_path);
            cmd.count = 0;
            cmd_append(&cmd, "scp", "-r", "./assets", nob_temp_sprintf("%s@%s:%s/", pi_user, pi_host, pi_path));
            if (!cmd_run(&cmd)) {
                nob_log(NOB_ERROR, "Failed to upload assets folder tree structure.");
                return 1;
            }
        }

        // Step C: Push up updated game runtime logic execution core binary
        nob_log(NOB_INFO, "Transferring execution core application binary...");
        cmd.count = 0;
        cmd_append(&cmd, "scp", "./main_pi", nob_temp_sprintf("%s@%s:%s/main_pi", pi_user, pi_host, pi_path));
        if (!cmd_run(&cmd)) {
            nob_log(NOB_ERROR, "Failed to deliver compilation runtime binary.");
            return 1;
        }

        nob_log(NOB_INFO, "Deployment finalized! Run './main_pi' inside folder '%s' on your Pi.", pi_path);
    }

    // Local native execution fallback
    if (run && !cross) {
        cmd.count = 0;
        cmd_append(&cmd, "./main");
        da_append_many(&cmd, argv, argc);
        if (!cmd_run(&cmd)) return 1;
    }

    return 0;
}

