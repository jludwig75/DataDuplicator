#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "DirectoryWalker.h"

#include "DataSource.h"
#include "DataDestination.h"


void printDirectory(ConstDirectoryPtr directory, size_t depth = 0)
{
    std::string padding(2 * depth, ' ');
    std::unique_ptr<RWLock::RLocker> lock;
    for (auto object : directory->contents(lock))
    {
        std::cout << padding << object->name();
        FileSystemObjectVisitor(
                [depth](ConstDirectoryPtr directory) {
                    std::cout << "\n";
                    printDirectory(directory, depth + 1);
                },
                [](ConstFilePtr file) {
                    std::cout << " - " << file->size() << " bytes\n";
                }
            ).visit(object);
    }
}

void walkDirs(std::string& dataRoot)
{
    DirectoryWalker walker;

    DirectoryPtr root;
    printf("Enumerating files in \"%s\"...\n", dataRoot.c_str());
    walker.start(dataRoot, root);
    printf("Contents of \"%s\":\n", root->path().c_str());
    printDirectory(root);
    walker.join();
}

int main(int argc, char* argv[])
{
    std::string programName = argv[0];
    std::vector<std::string> args(argv + 1, argv + argc);

    //walkDirs(args[0]);

    auto source = std::make_unique<DataSource>("f:/code");

    DataDestination destination(source.get(), "f:/code2");

    source->start();
    destination.start();

    source.reset();
    destination.stop();
}
