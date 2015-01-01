import os

LINEBREAKS = '\n\n\n\n'
DIRECTORIES = ['src/', 'src/jni', 'src/managers']


def prepend_license(filename, license):
    with open(filename, 'r+') as f:
        content = f.read()
        if content[0:len(license)] == license: return
        f.seek(0, 0)
        f.write(license.rstrip('\r\n') + LINEBREAKS + content)


def license_directory(directory, license):
    files = os.listdir(directory)
    files = [file for file in files if os.path.isfile(os.path.join(directory, file))]
    files = [file for file in files if (file[-4:] == '.cpp' or file[-2:] == '.h')]
    for file in files:
        prepend_license(os.path.join(directory,file), license)

if __name__ == "__main__":
    with open('license.txt', 'r') as license_file:
        license = license_file.read()
        license = "/*\n" + license + "\n*/"

    for directory in DIRECTORIES:
        license_directory('./'+directory, license)