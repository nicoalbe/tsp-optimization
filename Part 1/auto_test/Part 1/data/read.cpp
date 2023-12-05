#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

// Function to get file names in a directory
std::vector<std::string> GetFileNames(const std::string& directoryPath) {
    std::vector<std::string> fileNames;
    
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directoryPath + "\\*").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                fileNames.push_back(findFileData.cFileName);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }

    return fileNames;
}

int main() {
    std::ofstream MyFile("filenames.txt");
  
    std::string directoryPath = ".\\data";

    // Get file names
    std::vector<std::string> fileNames = GetFileNames(directoryPath);

    // Display file names
    std::cout << "Files in directory: " << directoryPath << std::endl;
    for (const auto& fileName : fileNames) {
        std::cout << fileName << std::endl;
        MyFile<<"\""<< fileName <<"\", ";
    }
MyFile.close();
  // Close the file
    return 0;
}
