import urllib.request
import os
import shutil
import zipfile
import tarfile
import sys
from subprocess import check_output
import subprocess

DIRNAME = 'dependencies'
DELETE_EXTRA = True


def dir_cleanup(name):
    if os.path.exists(name):
        shutil.rmtree(name)


def begin_dependency(name):
    print('Resolving dependency '+name)
    dir_cleanup('../'+name)
    dir_cleanup(name+'_all')


def end_dependency(name, source_directories, version):
    output_directory = '../'+name
    directory = name+'_all'

    if not os.path.exists(output_directory):
        os.makedirs(output_directory)
    for src_dir in source_directories:
        src_path = os.path.join(directory, src_dir)
        for f in os.listdir(src_path):
            file_path = os.path.join(src_path, f)
            if os.path.isfile(file_path):
                shutil.copy(file_path, os.path.join(output_directory, f))
            elif os.path.isdir(file_path):
                shutil.copytree(file_path, os.path.join(output_directory, f))

    with open(output_directory+'/VERSION', 'w') as f:
        f.write(version+"\n")
    if DELETE_EXTRA:
        shutil.rmtree(directory)
    print('Finished resolving dependency '+name)


def run_commands_in_directory(cmds, directory):
    if len(cmds) == 0:
        return
    prev_directory = os.getcwd()
    os.chdir(directory)
    for cmd in cmds:
        try:
            process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            while True:
                nextline = process.stdout.readline().decode()
                sys.stdout.write(nextline)
                if nextline == '' and process.poll() is not None:
                    print()
                    break
            print("Finished "+cmd)
        except subprocess.CalledProcessError:
            print('Warning: "'+cmd+'" had a non-0 return')

    os.chdir(prev_directory)


def github_dependency(url, name, source_directories, preparatory_commands=[]):
    begin_dependency(name)

    check_output(['git', 'clone', url])
    directory = url.split("/")[-1]
    os.rename(directory, name+'_all')
    directory = name+'_all'
    os.chdir(directory)
    version = check_output(['git', 'log', '-1', '--format==%cd']).decode()[1:-1] #This is hopefully portable
    os.chdir("..")

    run_commands_in_directory(preparatory_commands, directory)
    end_dependency(name, source_directories, version)


def download_dependency(url, name, source_directories, version, preparatory_commands=[], type=None):
    begin_dependency(name)

    if not type:
        type = '.'+url.split(".")[-1]
    urllib.request.urlretrieve(url, name+type)
    directory = name+'_all'
    if type == '.zip':
        with zipfile.ZipFile(name+'.zip') as zip:
            zip.extractall(directory)
        os.remove(name+'.zip')
    elif type == '.tar.gz':
        with tarfile.open(name+'.tar.gz') as trfile:
            trfile.extractall(directory)
        os.remove(name+'.tar.gz')

    run_commands_in_directory(preparatory_commands, directory)
    end_dependency(name, source_directories, version)



if __name__ == "__main__":

    #First, development dependencies.
    root_directory = os.getcwd();
    os.chdir("./src/lib/")    


    if not os.path.exists(DIRNAME):
        os.makedirs(DIRNAME)
    os.chdir(DIRNAME)

    download_dependency('https://docs.google.com/uc?export=download&id=0B9ZUy-jroUhzdGhwUUNhaFBXXzA', 'utf8cpp', ['source'],
                        '2.3.4', type='.zip')

    github_dependency('https://github.com/Mindful/tidy-html5', 'tidy-html5', ['lib', 'include'],
                      ['cd build/gmake/', 'make', 'cd ../..'])
    github_dependency('https://github.com/miloyip/rapidjson', 'rapidjson', ['include/rapidjson'])
    download_dependency('http://www.sqlite.org/2014/sqlite-amalgamation-3080600.zip', 'sqlite',
                        ['sqlite-amalgamation-3080600'], '3.8.6')
    download_dependency('http://github.com/zeux/pugixml/releases/download/v1.4/pugixml-1.4.zip', 'pugixml',
                        ['pugixml-1.4/src'], '1.4')
    #TODO: adjust the vmime one so it doesn't spit stuff out in disparate directories like it is now (see tidy-html5)
    github_dependency('https://github.com/Mindful/vmime', 'vmime', ['install'],
    ['cmake -G "Unix Makefiles" -DVMIME_HAVE_MESSAGING_PROTO_POP3=OFF -DVMIME_HAVE_MESSAGING_PROTO_SENDMAIL=OFF -DVMIME_HAVE_MESSAGING_PROTO_MAILDIR=OFF -DVMIME_HAVE_MESSAGING_PROTO_SMTP=OFF -DVMIME_SHARED_PTR_USE_CXX=ON -DVMIME_SHARED_PTR_USE_BOOST=OFF -DVMIME_BUILD_SHARED_LIBRARY=ON -DCMAKE_BUILD_TYPE="Release" -DVMIME_BUILD_SAMPLES=OFF -DVMIME_BUILD_STATIC_LIBRARY=OFF -DCMAKE_INSTALL_PREFIX=install'
     , 'make install'])

    download_dependency('https://googletest.googlecode.com/files/gtest-1.7.0.zip', 'gtest', ['gtest-1.7.0'], '1.7', ['cd gtest-1.7.0/ && cmake -G "Unix Makefiles" && make'])


