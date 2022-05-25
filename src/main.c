// This is basically a native C implementation of the following shell script:
//
// if ! grep 'pam_tid.so' /etc/pam.d/sudo --silent; then
//     sed -i -e '1s;^;auth       sufficient     pam_tid.so\n;' /etc/pam.d/sudo
// fi
//
// By implementing this operation as a native binary, we can allow it directly
// in Security & Privacy > Full Disk Access, as opposed to a shell script.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


const char *PAM_FILENAME = "/etc/pam.d/sudo";
const char *PAM_UPDATED_TEMP_FILE_NAME = "/tmp/persistent-touch-id-sudo-pam";


// Open file or exit with an error message.
FILE * OpenFileOrDie(const char *path, const char *mode) {
    FILE *fResult = fopen(path, mode);
    if (!fResult) {
        fprintf(stderr, "Error opening file %s (insufficient permissions?)", path);
        exit(1);
    }
    return fResult;
}


// Check if the PAM config for sudo already contains Touch ID configuration.
int IsPamAlreadyConfigured() {
    int result = 0;

    FILE *fPamConfig = OpenFileOrDie(PAM_FILENAME, "r");
    size_t bufferLen = 0;
    char *line = NULL;

    // Read PAM config line by line.
    while (getline(&line, &bufferLen, fPamConfig) != -1) {
        // If it already has the Touch ID line, consider it already configured.
        if (strstr(line, "pam_tid.so") != NULL) {
            result = 1;
            break;
        }
    }

    fclose(fPamConfig);
    if (line) {
        free(line);
    }

    return result;
}


// Copy the contents of one file to another, overwriting the target.
void CopyFile(const char *sourcePath, const char *destinationPath) {
    FILE *fSource = OpenFileOrDie(sourcePath, "r");
    FILE *fDestination = OpenFileOrDie(destinationPath, "w");

    size_t bufferLen = 0;
    char *line = NULL;

    fprintf(stdout, "Copying %s to %s\n", sourcePath, destinationPath);

    // Copy file contents line by line.
    while (getline(&line, &bufferLen, fSource) != -1) {
        fputs(line, fDestination);
    }

    fclose(fSource);
    fclose(fDestination);
}


// Generate a new PAM config to a temporary file.
void GeneratePamConfig() {
    const char *tidConfigString = "auth       sufficient     pam_tid.so\n";

    FILE *fOldPamConfig = OpenFileOrDie(PAM_FILENAME, "r");
    FILE *fNewPamConfig = OpenFileOrDie(PAM_UPDATED_TEMP_FILE_NAME, "w");

    size_t bufferLen = 0;
    char *line = NULL;
    int lineCounter = 0;

    fprintf(stdout, "Generating %s\n", PAM_UPDATED_TEMP_FILE_NAME);

    // Read PAM config line by line.
    while (getline(&line, &bufferLen, fOldPamConfig) != -1) {
        lineCounter++;
        // Insert the new line after the comment header.
        if (lineCounter == 1) {
            if (strstr(line, "#")) {
                // Copy the header comment first, then add Touch ID.
                fputs(line, fNewPamConfig);
                fputs(tidConfigString, fNewPamConfig);
            } else {
                // No header: just add Touch ID as the first line.
                fputs(tidConfigString, fNewPamConfig);
                fputs(line, fNewPamConfig);
            }
        } else {
            fputs(line, fNewPamConfig);
        }
    }

    fclose(fNewPamConfig);
    fclose(fOldPamConfig);
}


// Add Touch ID configuration to PAM config for sudo.
void ConfigurePam() {
    GeneratePamConfig();
    CopyFile(PAM_UPDATED_TEMP_FILE_NAME, PAM_FILENAME);
}


int main() {
    if (IsPamAlreadyConfigured()) {
        fprintf(stdout, "Nothing to do: PAM already configured for Touch ID\n");
    } else {
        fprintf(stdout, "Adding Touch ID PAM configuration\n");
        ConfigurePam();
    }
    return 0;
}
