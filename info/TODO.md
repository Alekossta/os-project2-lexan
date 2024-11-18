./lexan -i ./data/texts/GreatExpectations_a.txt -l 50 -m 50 -t 5 -e ./data/exclusionLists/exclusionList1_a.txt -o output.txt

exec*()
mkfifo()
pipe()
open()
close()
read()
write()
wait()
waitpid()
poll()
select()
dup()
dup2()
kill()
exit()

ExclusionList το αρχείο με τις λέξεις που ΔΕΝ θα πρέπει να περάσουν για επεξεργασία στους builders,

OutputFile το αρχείο που ο root θα τυπώσει όλες τις λέξεις που βρέθηκαν στην διάρκεια της ανάλυσης
μαζί με τους σχετικούς counters.

μία τέτοια ιεραρχία της οποίας
οι κόμβοι συνολικά διαβάζουν κομμάτια αρχείου κειμένου τυχαίου μεγέθους, επιτελούν λεξικογραφική ανάλυση,
και από κοινού παράγουν τις top-k μη–συνήθεις λέξεις που βρίσκουν στο εν λόγω αρχείο κειμένου.