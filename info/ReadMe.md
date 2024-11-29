# Folder Structure Description
/bin εχει απλα τα executables μεσα
/data εχει μεσα δεδομενα
    /data/exclusionsLists .txt αρχεια exclusion list
    /data/output φακελος για output files
    /data/texts τα texts που θα αναλυσουμε
/include εχει μεσα .h αρχεια
/info info αρχεια
/obj .o αρχεια
/src .c αρχεια

# Utility Αρχεία

## Console Reader
ενα απλο abstraction για να παιρνουμε τα arguments που περναμε απο το 
command line με την κληση μια συναρτησης readConsole που γυρναει ενα 
struct ConsoleArguments.

## Hashtable
μια δομη δεδομενων hashtable στατικου μεγεθους με buckets με linked list
και hash function. Εχει καποιες επιπλεον functions που δεν θα βλεπαμε
σε τυπικο hashtable implementation για την εργασια αυτην συγκεκριμενα

# Βασικά Αρχεία

## lexan
1. διαβαζουμε τα console arguments με το abstraction του ConsoleReader
2. μετραμε ποσες γραμμες εχει το input file
3. φτιαχνουμε N pipes για splitter to builder communication, οπου
N ειναι ο αριθμος των builders. Αρα καθε builder θα εχει ενα pipe
4. Ακουμε για signals USR1 που θα ερθουν οταν τελειωσει ο splitter
5. Υπολογιζω τις γραμμες που θα διαβασει καθε splitter. Σε περιπτωση που εχουμε υπολοιπο
στην διαιρεση κανουμε ενα loop και αναθετουμε μια μια γραμμες σε καθε splitter μεχρι να
ξεμεινουμε απο γραμμες
6. Επειτα κανω το spawn των splitters. Κλεινω ολα τα pipe ends που δεν χρειαζομαι και περναω οσα χρειαζομαι στις θεσεις +3 για να εχουν access οι splitters.
7. Επειτα εκτελω το αρχειο splitter και περναω arguments τα εξης. Τον αριθμο των γραμμων
που θα διαβασει καθε splitter. Την πρωτη γραμμη που θα διαβασει καθε splitter. Τον αριθμο
των builders που θα εχουμε.
8. Επειτα φτιαχνω τα pipes για την επικοινωνια απο builder σε root.
9. Μετα κανω setup να ακουω για events απο το USR2 οταν τελειωνει/πεθαινει το builder.
10. Μετα κανω spawn τους builders και κλεινω οσα pipe ends δεν χρειαζομαι. Κανω μονο
dup2 το pipe στον fd stdin για να μπορουν οι builders να κανουν read.
11. Μετα εκτελω το builder και περναω μονο τον αριθμο των builders σαν argument
12. Επισης περναω στο pollfd array τα fds και θετω τα events type να ειναι POLIN 
για να ελεγχουμε αμα το i-οστο fd θα το ελεγχουμε για διαβασμα
13. Μετα φτιαχνω ενα hashtable που θα περνω τις λεξεις που θα στελνονται στο root
14. Μετα φτιαχνω ενα struct για να αποθηκευω δεδομενα για καθε buffer
15. Μετα κανουμε poll μεχρι να εχουν τελειωσει ολοι οι builders (-1 σημαινει οτι
περιμενουμε για παντα μεχρι να χτυπησει event)
16. Μετα κανουμε iterate σε ολους τους builders για να δουμε αμα ειναι ετοιμος καποιος
builder να κανει write (POLLIN). Αν εχουμε error (POLLERR) η τελειωσε (POLLHUP) τοτε
το κλεινουμε.
