mkdir -p "${HOME}/.thoriumengine/1.0"
DIR="$( cd "$( dirname "$0" )" && pwd )"
file="${HOME}/.thoriumengine/1.0/path.txt"
echo $DIR > $file
cat $file
