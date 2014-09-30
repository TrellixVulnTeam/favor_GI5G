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


def github_dependency(url, name, source_directory, preparatory_commands=[]):
    print('Resolving dependency '+name)
    dir_cleanup('../'+name)
    dir_cleanup(name+'_all')

    check_output(['git', 'clone', url])
    directory = url.split("/")[-1]

    os.rename(directory, name+'_all')
    directory = name+'_all'

    os.chdir(directory)
    for cmd in preparatory_commands:
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

    version = check_output(['git', 'log', '-1', '--format==%cd']).decode()[1:-1] #This is hopefully portable
    with open(source_directory+'/VERSION', 'w') as f:
        f.write(version+"\n")
    os.chdir("..")

    shutil.move(directory+'/'+source_directory, '../'+name)
    if DELETE_EXTRA:
        shutil.rmtree(directory)
    print('Finished resolving dependency '+name)


def download_dependency(url, name, source_directory, version, prepatory_commands=[]):
    print('Resolving dependency '+name)
    dir_cleanup('../'+name)
    dir_cleanup(name+'_all')

    #TODO: we should move into the directory and run preparatory commands at some point

    type = '.'+url.split(".")[-1]
    urllib.request.urlretrieve(url, name+type)
    if type == '.zip':
        with zipfile.ZipFile(name+'.zip') as zip:
            zip.extractall(name+'_all')
        os.remove(name+'.zip')
    elif type == '.tar.gz':
        with tarfile.open(name+'.tar.gz') as trfile:
            trfile.extractall(name+'_all')
        os.remove(name+'.tar.gz')

    shutil.move(name+'_all/'+source_directory, '../'+name)
    with open('../'+name+'/VERSION', 'w') as f:
        f.write(version+"\n")

    if DELETE_EXTRA:
        shutil.rmtree(name+'_all')
    print('Finished resolving dependency '+name)



if __name__ == "__main__":
    if not os.path.exists(DIRNAME):
        os.makedirs(DIRNAME)
    os.chdir(DIRNAME)

    github_dependency('https://github.com/miloyip/rapidjson', 'rapidjson', 'include/rapidjson')
    download_dependency('http://www.sqlite.org/2014/sqlite-amalgamation-3080600.zip', 'sqlite',
                        'sqlite-amalgamation-3080600', '3.8.6')
    download_dependency('http://github.com/zeux/pugixml/releases/download/v1.4/pugixml-1.4.zip', 'pugixml',
                        'pugixml-1.4/src', '1.4')
    github_dependency('https://github.com/Mindful/vmime', 'vmime', 'install',
    ['cmake -G "Unix Makefiles" -DVMIME_HAVE_MESSAGING_PROTO_POP3=OFF -DVMIME_HAVE_MESSAGING_PROTO_SENDMAIL=OFF -DVMIME_HAVE_MESSAGING_PROTO_MAILDIR=OFF -DVMIME_HAVE_MESSAGING_PROTO_SMTP=OFF -DVMIME_SHARED_PTR_USE_CXX=ON -DVMIME_SHARED_PTR_USE_BOOST=OFF -DVMIME_BUILD_SHARED_LIBRARY=ON -DCMAKE_BUILD_TYPE="Release" -DVMIME_BUILD_SAMPLES=OFF -DVMIME_BUILD_STATIC_LIBRARY=OFF -DCMAKE_INSTALL_PREFIX=install'
     , 'make install'])


