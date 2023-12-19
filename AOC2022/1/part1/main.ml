let get_next_line () = 
  try
    Some (read_line ())
  with
    End_of_file -> None

let rec get_next_elf_rec elves =
  match get_next_line () with
  | None | Some "" -> elves
  | Some x -> get_next_elf_rec (int_of_string x :: elves)

let get_next_elf () = get_next_elf_rec []

let rec add_up_calories_rec elf calories =
  match elf with
  | [] -> calories
  | x :: rest -> add_up_calories_rec rest calories + x

let add_up_calories elf = add_up_calories_rec elf 0

let get_next_elf_calories () =
  match get_next_elf () with
  | [] -> None
  | elf -> Some (add_up_calories elf)

let rec get_calories_list_rec list = 
  match get_next_elf_calories () with
  | None -> list
  | Some x -> get_calories_list_rec (x :: list)

let get_calories_list () = get_calories_list_rec []

let rec get_list_max_rec list max = 
  match list with
  | [] -> max
  | head :: rest -> get_list_max_rec rest (if head > max then head else max)

let get_list_max list =
  match list with
  | [] -> None
  | head :: rest -> Some (get_list_max_rec rest head)

let get_max_calories () =
  match get_list_max (get_calories_list ()) with
  | None -> raise Exit
  | Some x -> x

let () =
  try
    print_string "Max calories = "; print_int (get_max_calories ()); print_string "\n"
  with
    Exit -> print_endline "ERROR: Invalid inputs file"
