#!/bin/sh
__AUTHOR__="Maxx Boehme"
# Preconditions:
# - Packages doxygen doxygen-doc doxygen-latex doxygen-gui graphviz
#   must be installed to create Doxygen files.
# - A gh-pages branch should already exist. See below for more info on how to
#   create a gh-pages branch.
#
# Required global variables:
# - TRAVIS_BUILD_NUMBER : The number of the current build.
# - TRAVIS_COMMIT       : The commit that the current build is testing.
# - GH_REPO_NAME        : The name of the repository.
# - GH_REPO_REF         : The GitHub reference to the repository.
# - GH_REPO_TOKEN       : Secure token to the github repository.

echo 'Setting up the script...'
# Exit with nonzero exit code if anything fails
set -e

# Checking that we are on master branch as do not want to update documentation
# if on a different branch.
BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [[ "$BRANCH" != "master" ]]; then
    echo 'Aborting script as not on master branch: ${BRANCH}';
    exit 1
fi

mkdir -p code_docs
cd code_docs

git clone -b gh-pages https://git@$GH_REPO_REF
cd $GH_REPO_NAME

##### Configure git
# Set the push default to simple i.e. push only the current branch.
# git config --global push.default simple
# Pretend to be a user called Travis CI.
# git config user.name "Travis CI"
# git config user.email "travis@travis-ci.org"

# Remove everything currently in the gh-pages doxygen folder.
# gitHub is smart enough to know which files have changed ans which files have
# stayed the same and will only update the changed files. So the gh-pages branch
# can be safely cleaned, and it is sure that everything pushed later is the new
# documentation
rm -rf ./doxygen
mkdir -p ./doxygen

# Copying doxygen output into gh-pages directory.
cp -r ../../Documentation/* ./doxygen/

##### Upload the documentation to the gh-pages branch
# Only upload if Doxygen successfully created the documentation.
# Check this by verifying that the html directory and the file html/index.html
# both exist. This is a good indication that Doxygen did it's work.
if [ -d "doxygen/html" ] && [ -f "doxygen/html/index.html" ]; then
    echo 'Uploading documentation to the gh-pages branch...'
    # Add everything in this directory (the Doxygen code documentation) to the
    # gh-pages branch.
    git add --all

    # Commit the added files with a title and description containing the Travis CI
    # build number and the GitHub commit reference that issued this build.
    git commit -m "Deploy Doxygen docs to GitHub Pages from Travis build: ${TRAVIS_BUILD_NUMBER}" -m "Commit: ${TRAVIS_COMMIT}" -m "Message: ${TRAVIS_COMMIT_MESSAGE}"

    # Force push to the remote gh-pages
    git push --force "https://${GH_REPO_TOKEN}@${GH_REPO_REF}" > /dev/null 2>&1
else
    echo '' >&2
    echo 'Warning: No documenation (html) files have been found!' >&2
    echo 'Warning: Not going to push the documentation to GitHub!' >&2
    exit 1
fi

