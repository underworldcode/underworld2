#! /usr/bin/env bash


set -e
cd ${1}/user_guide
find . -name \*.html | xargs sed -i -e 's/<\/body>//g'
find . -name \*.html | xargs sed -i -e 's/<\/html>//g'
# make bash variable to append to html
VAR=$(cat <<'END_HEREDOC'
        <script src="../_static/juniper.min.js"></script>
        <script>
            new Juniper({
                repo: 'underworldcode/underworld2',
                branch: 'GITCOMMIT',
                isolateCells: false,
                theme: 'material',
                msgLoading: "Done"
            });
        </script>
</body>
<head>
  <link rel="stylesheet" href="../_static/css/material.css" />
</head>
</html>
END_HEREDOC
)

echo "$VAR" | tee -a *.html >/dev/null

# get current git commit
export COMMIT=`git rev-parse --verify HEAD`
find . -name \*.html | xargs sed -i "s/GITCOMMIT/${COMMIT}/g"
