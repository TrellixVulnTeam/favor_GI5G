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
            print("Run "+cmd)
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
    if directory[-4:] == ".git":
        directory = directory[:-4]
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
            def is_within_directory(directory, target):
                
                abs_directory = os.path.abspath(directory)
                abs_target = os.path.abspath(target)
            
                prefix = os.path.commonprefix([abs_directory, abs_target])
                
                return prefix == abs_directory
            
            def safe_extract(tar, path=".", members=None, *, numeric_owner=False):
            
                for member in tar.getmembers():
                    member_path = os.path.join(path, member.name)
                    if not is_within_directory(path, member_path):
                        raise Exception("Attempted Path Traversal in Tar File")
            
                tar.extractall(path, members, numeric_owner=numeric_owner) 
                
            
            safe_extract(trfile, directory)
        os.remove(name+'.tar.gz')

    run_commands_in_directory(preparatory_commands, directory)
    end_dependency(name, source_directories, version)



if __name__ == "__main__":
    root_directory = os.getcwd();
    os.chdir("./src/lib/")    


    if not os.path.exists(DIRNAME):
        os.makedirs(DIRNAME)
    os.chdir(DIRNAME)


    #We're adding -fPIC to several libraries here (vmime, tidy-html5, etc.) so that they
    #can be linked statically as it's cleaner this way

    #SASL support is turned off for now because it introduces more dependencies
    #The TLS_SUPPORT_LIB and CHARSETCONV_LIB could conceivably be changed, but
    #at least the former would also requires changes to the cmake file
    github_dependency('https://github.com/Mindful/vmime', 'vmime', ['install'],
    ['cmake -G "Unix Makefiles" -DCMAKE_CXX_FLAGS="${CMAKE_C_FLAGS} -fPIC" '
    '-DVMIME_HAVE_MESSAGING_PROTO_POP3=OFF -DVMIME_HAVE_MESSAGING_PROTO_SENDMAIL=OFF '
    '-DVMIME_HAVE_MESSAGING_PROTO_MAILDIR=OFF -DVMIME_HAVE_MESSAGING_PROTO_SMTP=OFF '
    '-DVMIME_SHARED_PTR_USE_CXX=ON -DVMIME_SHARED_PTR_USE_BOOST=OFF -DVMIME_BUILD_SHARED_LIBRARY=OFF'
    '-DCMAKE_BUILD_TYPE="Release" -DVMIME_BUILD_SAMPLES=OFF -DVMIME_BUILD_STATIC_LIBRARY=ON '
    '-DVMIME_HAVE_SASL_SUPPORT=OFF -DVMIME_TLS_SUPPORT_LIB=openssl -DVMIME_CHARSETCONV_LIB=iconv '
    '-DCMAKE_INSTALL_PREFIX=install' , 'make install'])

    github_dependency('https://github.com/htacg/tidy-html5', 'tidy-html5', ['build/cmake', 'include'],
                      ['cd build/cmake && cmake ../.. -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -fPIC" '
                      '-DBUILD_SHARED_LIB:BOOL=OFF' '-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../include'
                      ' && make && make install'])

    github_dependency('https://github.com/miloyip/rapidjson', 'rapidjson', ['include/rapidjson'])
    github_dependency('https://github.com/zeux/pugixml.git', 'pugixml', ['src'])


    #Cmake for this may be redundant because it's in our cmake files anyway, but it doesn't hurt
    github_dependency('https://github.com/google/googletest', 'gtest', ['.'], ['cmake -G "Unix Makefiles" && make'])


    github_dependency('https://github.com/unnonouno/iconvpp', 'iconvpp', ['.'])

    download_dependency('https://docs.google.com/uc?export=download&id=0B9ZUy-jroUhzdGhwUUNhaFBXXzA', 'utf8cpp', ['source'],
                        '2.3.4', type='.zip')
    download_dependency('https://www.sqlite.org/2015/sqlite-amalgamation-3090200.zip', 'sqlite',
                        ['sqlite-amalgamation-3090200'], '3.9.2')


