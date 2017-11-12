<?php
class Table
{
    private static $tableList;

    private static $tableArray;

    public static function Load()
    {
        $dir = './json';
        $dh = opendir($dir);
        while (($file = readdir($dh)) !== false)
        {
            if (substr($file, 0, 1) != '.')
            {
                self::$tableList[substr($file, 0, -5)] = json_decode(file_get_contents("./json/$file"));
                self::$tableArray[substr($file, 0, -5)] = json_decode(file_get_contents("./json/$file"), true);
            }
        }
        closedir($dh);
    }

    public static function List($tableName)
    {
        return self::$tableList[$tableName];
    }

    public static function Array($tableName)
    {
        return self::$tableArray[$tableName];
    }

    public static function Rand($tableName, $number)
    {
        return array_rand(self::$tableArray[$tableName], $number);
    }

    public static function Get($tableName, $key)
    {
        return @self::$tableList[$tableName]->{$key};
    }

    public static function ArrayGet($tableName, $key)
    {
        return @self::$tableArray[$tableName][$key];
    }

    public static function RandGet($tableName)
    {
        $table = self::$tableArray[$tableName];
        return $table[array_rand($table)];
    }

    public static function Find($tableName, $arrayKey)
    {
        foreach(self::$tableList[$tableName] as $tableValue)
        {
            $find = true;
            foreach($arrayKey as $key =>$keyValue)
            {
                if ($tableValue->{$key} != $keyValue)
                {
                    $find = false;
                    break;
                }
            }

            if ($find)
            {
                return $tableValue;
            }
        }
    }

    public static function FindArray($tableName, $arrayKey)
    {
        $result = null;
        foreach(self::$tableList[$tableName] as $tableValue)
        {
            $find = true;
            foreach($arrayKey as $key =>$keyValue)
            {
                if ($tableValue->{$key} != $keyValue)
                {
                    $find = false;
                    break;
                }
            }

            if ($find)
            {
                $result[] = $tableValue;
            }
        }

        return $result;
    }

    public static function Count($tableName, $arrayKey)
    {
        return count(self::FindArray($tableName, $arrayKey));
    }

    public static function Has($tableName, $key)
    {
        return isset($tableName[$key]);
    }

    public static function HasMatch($tableName, $arrayKey)
    {
        return self::Find($tableName, $arrayKey) != null;
    }

    public static function Test()
    {
        self::Load();

        print("List---------------------\n");
        print_r(self::List('hero'));

        print("Array---------------------\n");
        print_r(self::Array('hero'));

        print("Rand---------------------\n");
        print_r(self::Rand('hero', 1));
        print("\nRand---------------------\n");
        print_r(self::Rand('hero', 2));

        print("RandGet---------------------\n");
        print_r(self::RandGet('hero'));

        print("Get---------------------\n");
        print_r(self::Get('hero', 3));
        var_dump(self::Get('hero', 6));

        print("ArrayGet---------------------\n");
        print_r(self::ArrayGet('hero', 3));
        var_dump(self::ArrayGet('hero', 6));

        print("Find---------------------\n");
        print_r(self::Find('hero', ['rare' => 10, 'type' => 2]));
        var_dump(self::Find('hero', ['rare' => 10, 'type' => 6]));

        print("FindArray---------------------\n");
        print_r(self::FindArray('hero', ['rare' => 10, 'type' => 1]));
        var_dump(self::FindArray('hero', ['rare' => 10, 'type' => 6]));

        print("Count---------------------\n");
        var_dump(self::Count('hero', ['rare' => 10, 'type' => 1]));
        var_dump(self::Count('hero', ['rare' => 10, 'type' => 6]));

        print("Has---------------------\n");
        var_dump(self::Has('hero', 3));
        var_dump(self::Has('hero', 6));

        print("HasMatch---------------------\n");
        var_dump(self::HasMatch('hero', ['rare' => 10, 'type' => 1]));
        var_dump(self::HasMatch('hero', ['rare' => 10, 'type' => 6]));
    }
}
//Table::Test();
?>
